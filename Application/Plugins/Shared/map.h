
#ifndef FLAG_MAP
#define FLAG_MAP

namespace SOFTIMAGE {

class instance_map_operator									// this is my custom scene graph handler
{
public:
	CString name;											// unique name
	virtual void Init() = 0;								// init dataset
	virtual void Delete() = 0;								// delete dataset
};

struct instance_map
{
	instance_map_operator *op;
	instance_map *next;
};

class instance_map_manager
{
public:
	instance_map_manager() {}
	~instance_map_manager() {}

	instance_map *map;

	void Init()
	{
		map = NULL;
	}

	instance_map_operator* Get(CString in_name)							// get pointer
	{
		instance_map_operator* pointer = NULL;

		instance_map *pOriginal = map;
		instance_map *pBrowser = map;
		instance_map *pNew_entry = NULL;

		while(pBrowser)
		{
			if (pBrowser->op)
			{
				if (pBrowser->op->name == in_name)
				{
					pointer = pBrowser->op;
					pBrowser = NULL;
				}
				else pBrowser = pBrowser->next;
			}
			else pBrowser = pBrowser->next;
		}

		return pointer;
	}

	void Add_new(CString in_name, instance_map_operator* pointer)		// add pointer
	{
		instance_map *pOriginal = map;
		instance_map *pNew_entry = NULL;

		pNew_entry = new instance_map;
		pNew_entry->op = pointer;

		pointer->name = in_name;
		pointer->Init();

		pNew_entry->next = pOriginal;
		map = pNew_entry;
	}

	void Delete(CString in_name)
	{
		instance_map *pBrowser = map;
		instance_map *pPrev = NULL;

		while(pBrowser)
		{
			if (pBrowser->op)
			{
				if (pBrowser->op->name == in_name)
				{
					pBrowser->op->Delete();
					delete pBrowser->op;

					if (pPrev)
						pPrev->next = pBrowser->next;
					else
						map = pBrowser->next;

					delete pBrowser;
					pBrowser = NULL;
				}
				else
				{
					pPrev = pBrowser;
					pBrowser = pBrowser->next;
				}
			}
			else
			{
				pPrev = pBrowser;
				pBrowser = pBrowser->next;
			}
		}
	}
/*
	void Render_all(CGraphicSequencer& oSequencer)	// render every entry in the map
	{
		b_instance_map *pBrowser = map;
		while(pBrowser)
		{
			if (pBrowser->op)
			{
				pBrowser->op->Render(oSequencer);
			}
			pBrowser = pBrowser->next;
		}
	}
*/
	void Wipe()										// delete every entry in the map
	{
		instance_map *pBrowser = map;
		instance_map *pPrev = NULL;

		while(pBrowser)
		{
			if (pBrowser->op)
			{
				pBrowser->op->Delete();
				pBrowser->op;

				if (pPrev)
					pPrev->next = pBrowser->next;
				else
					map = pBrowser->next;

				delete pBrowser;
				pBrowser = NULL;
			}
			else
			{
				pPrev = pBrowser;
				pBrowser = pBrowser->next;
			}
		}

		map = NULL;
	}

};

} // namespace SOFTIMAGE


#endif // FLAG_MAP
