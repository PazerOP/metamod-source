/* ======== SourceMM ========
* Copyright (C) 2004-2005 SourceMM Development Team
* No warranties of any kind
*
* License: zlib/libpng
*
* Author(s): David "BAILOPAN" Anderson
* ============================
*/

#include "CPlugin.h"
#include "CISmmAPI.h"

/** 
 * @brief Implements functions from CPlugin.h
 * @file CPlugin.cpp
 */

using namespace SourceMM;

CPluginManager g_PluginMngr;

CPluginManager::CPluginManager()
{
	m_LastId = Pl_MinId;
}

CPluginManager::~CPluginManager()
{
	if (m_Plugins.size())
	{
		UnloadAll();
		m_Plugins.clear();
	}
}

CPluginManager::CPlugin::CPlugin() : m_Lib(NULL), m_API(NULL), m_Id(0), m_Source(0)
{
}

PluginId CPluginManager::Load(const char *file, PluginId source, bool &already, char *error, size_t maxlen)
{
	PluginIter i;

	already = false;
	//Check if we're about to reload an old plugin
	for (i=m_Plugins.begin(); i!=m_Plugins.end(); i++)
	{
		if ( (*i) && (*i)->m_File.compare(file)==0 )
		{
			if ( (*i)->m_Status < Pl_Paused )
			{
				//Attempt to load the plugin again
				already = true;
				i = m_Plugins.erase(i);
			} else {
				//No need to load it
				already = true;
				return (*i)->m_Id;
			}
		}
	}

	CPlugin *pl = _Load(file, source, error, maxlen);

	if (!pl)
		return Pl_BadLoad;

	return pl->m_Id;
}

CPluginManager::CPlugin *CPluginManager::FindById(PluginId id)
{
	PluginIter i;

	for (i=m_Plugins.begin(); i!=m_Plugins.end(); i++)
	{
		if ( (*i)->m_Id == id )
			return (*i);
	}

	return NULL;
}

bool CPluginManager::Pause(PluginId id, char *error, size_t maxlen)
{
	CPlugin *pl = FindById(id);
	
	if (!pl)
	{
		snprintf(error, maxlen, "Plugin id not found");
		return false;
	}

	return _Pause(pl, error, maxlen);
}

bool CPluginManager::Unpause(PluginId id, char *error, size_t maxlen)
{
	CPlugin *pl = FindById(id);
	
	if (!pl)
	{
		snprintf(error, maxlen, "Plugin id not found");
		return false;
	}

	return _Unpause(pl, error, maxlen);
}

bool CPluginManager::Unload(PluginId id, char *error, size_t maxlen)
{
	CPlugin *pl = FindById(id);
	
	if (!pl)
	{
		snprintf(error, maxlen, "Plugin id not found");
		return false;
	}

	return _Unload(pl, error, maxlen);
}

CPluginManager::CPlugin *CPluginManager::_Load(const char *file, PluginId source, char *error, size_t maxlen)
{
	FILE *fp;
	CPlugin *pl;

	pl = new CPlugin();
	*error = '\0';
	
	//Add plugin to list
	pl->m_Id = m_LastId;
	pl->m_File.assign(file);
	m_Plugins.push_back(pl);
	m_LastId++;

	//Check if the file even exists
	fp = fopen(file, "r");
	if (!fp)
	{
		if (error)
			snprintf(error, maxlen, "File not found: %s", file);
		pl->m_Status = Pl_NotFound;
	} else {
		fclose(fp);
		fp = NULL;
		
		//Load the file
		pl->m_Lib = dlmount(file);
		if (!pl->m_Lib)
		{
			if (error)
				snprintf(error, maxlen, "%s", dlerror());
			pl->m_Status = Pl_Error;
		} else {
			CreateInterfaceFn pfn = reinterpret_cast<CreateInterfaceFn>(dlsym(pl->m_Lib, PL_EXPOSURE_C));
			if (!pfn)
			{
				if (error)
					snprintf(error, maxlen, "Function %s not found", PL_EXPOSURE_C);
				pl->m_Status = Pl_Error;
			} else {
				pl->m_API = static_cast<ISmmPlugin *>((pfn)(PLAPI_NAME, NULL));
				if (!pl->m_API)
				{
					if (error)
						snprintf(error, maxlen, "Failed to get API");
					pl->m_Status = Pl_Error;
				} else {
					if (pl->m_API->GetApiVersion() < PLAPI_MIN_VERSION)
					{
						if (error)
							snprintf(error, maxlen, "Plugin API %d is out of date with required minimum (%d)", pl->m_API->GetApiVersion(), PLAPI_MIN_VERSION);
						pl->m_Status = Pl_Error;
					} else {
						if (pl->m_API->Load(pl->m_Id, static_cast<ISmmAPI *>(&g_SmmAPI), &(pl->fac_list), error, maxlen))
						{
							pl->m_Status = Pl_Running;
						} else {
							pl->m_Status = Pl_Refused;
						}
					}
				}
			}
		}
	}

	if (pl->m_Lib && (pl->m_Status < Pl_Paused))
	{
		dlclose(pl->m_Lib);
		pl->m_Lib = NULL;
		pl->m_API = NULL;
	}

	return pl;
}

bool CPluginManager::_Unload(CPluginManager::CPlugin *pl, char *error, size_t maxlen)
{
	if (error)
		*error = '\0';
	if (pl->m_API && pl->m_Lib)
	{
		if (pl->m_API->Unload(error, maxlen))
		{
			dlclose(pl->m_Lib);
			pl->m_Lib = NULL;
			pl->m_API = NULL;

			//Remove the plugin from the list
			PluginIter i;
			for (i=m_Plugins.begin(); i!=m_Plugins.end(); i++)
			{
				if ( (*i)->m_Id == pl->m_Id )
				{
					i = m_Plugins.erase(i);
					break;
				}
			}
			delete pl;

			return true;
		}
	} else {
		//The plugin is not valid, and let's just remove it from the list anyway
		PluginIter i;
		for (i=m_Plugins.begin(); i!=m_Plugins.end(); i++)
		{
			if ( (*i)->m_Id == pl->m_Id )
			{
				i = m_Plugins.erase(i);
				break;
			}
		}
		delete pl;

		return true;
	}

	return false;
}

bool CPluginManager::_Pause(CPluginManager::CPlugin *pl, char *error, size_t maxlen)
{
	if (error)
		*error = '\0';
	if (pl->m_Status != Pl_Running || !pl->m_API)
	{
		if (error)
			snprintf(error, maxlen, "Plugin cannot be paused");
	} else {
		return pl->m_API->Pause(error, maxlen);
	}

	return false;
}

bool CPluginManager::_Unpause(CPluginManager::CPlugin *pl, char *error, size_t maxlen)
{
	if (error)
		*error = '\0';

	if (pl->m_Status != Pl_Paused || !pl->m_API)
	{
		if (error)
			snprintf(error, maxlen, "Plugin cannot be unpaused");
	} else {
		return pl->m_API->Unpause(error, maxlen);
	}

	return false;
}


bool CPluginManager::UnloadAll()
{
	PluginIter i;
	bool status = true;

	for (i=m_Plugins.begin(); i!=m_Plugins.end(); i++)
	{
		if ( (*i) )
		{
			if ( (*i)->m_API )
			{
				if ( (*i)->m_API->Unload(NULL, 0) )
					status = false;

				dlclose( (*i)->m_Lib );
			}
			delete (*i);
		}

		i = m_Plugins.erase(i);
	}

	return status;
}

bool CPluginManager::Query(PluginId id, const char *&file, factories *&list, Pl_Status &status, PluginId &source)
{
	CPlugin *pl = FindById(id);

	if (!pl)
		return false;

	file = pl->m_File.c_str();
	list = &(pl->fac_list);
	status = pl->m_Status;
	source = pl->m_Source;

	return true;
}

PluginIter CPluginManager::_begin()
{
	return m_Plugins.begin();
}

PluginIter CPluginManager::_end()
{
	return m_Plugins.end();
}