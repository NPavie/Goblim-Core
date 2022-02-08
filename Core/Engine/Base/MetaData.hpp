/*
 *	Meta data clas for registering and creating Materials
 *
 */
#ifndef _METAMGR_MGR
#define _METAMGR_MGR

#include <map>
#include <iostream>
using namespace std;



class Metadata
{
public:
 Metadata(const char* name) : m_Name(name) {}

  const char* name() const { return m_Name; }

private:
  const char* m_Name;
};

template <typename Metatype>
class MetaCreator
{
public:

  MetaCreator( std::string name, unsigned size )
  {
    Init( name, size );
  }



  static void Init( std::string name, unsigned size )
  {
    Get( )->Init( name, size );
  }
  // Ensure a single instance can exist for this class type

  static MetaData *Get( void )
  {
    static MetaData instance;
    return &instance;
  }

};


class MetaManager
{
public:
  // add a new Metadata instance to the manager
  static void registerMeta(const Metadata* meta)
  {
    MetaMap& metas = getMetas();
    metas[meta->name()] = meta;
  }

  // find an instance of a Metadata object by name
  static const Metadata* get(const char* name)
  {
    const MetaMap& metas = getMetas();
    MetaMap::const_iterator meta = metas.find(name);
    return meta == metas.end() ? NULL : meta->second;
  }

private:
  typedef std::map<std::string, const Metadata*> MetaMap;

  // safe and easy singleton for our std::map of Metadata objects
  static MetaMap& getMetas()
  {
    static MetaMap metas;
    return metas;
  }
};

#define DECLARE_META(metatype) extern Metadata g_Meta ## metatype;
#define DEFINE_META(metatype) Metadata g_Meta ## metatype(#metatype, sizeof(metatype));
#define META_TYPE(metatype) (&g_Meta ## metatype)
#define META(object) ((object)->getMetadata())

#define DYNAMIC_META(metatype) public: const Metadata* getMetadata() const { return META_TYPE(metatype); }


/*
template <typename MetaType>
class MetaSingleton
{
public:
  static const Metadata* get() { return &s_Meta; }

private:
  static Metadata s_Meta;
};

#define DEFINE_META(metatype) Metadata MetaSingleton<metatype>::s_Meta(#metatype, sizeof(metatype));
#define META_TYPE(metatype) (MetaSingleton<metatype>::get())
#define META(object) (MetaSingleton<decltype(object)>::get())
*/



#endif