#ifndef __GLIP_NAMINGLAYOUT__
#define __GLIP_NAMINGLAYOUT__

    // Includes
        #include <string>
        #include <vector>
        #include <list>
        #include "Exception.hpp"

    // Constants
        #define DEFAULT_NAME "No Name\0"
        #define NO_PORT      (-1)
        #define SEPARATOR    "::\0"
        #define BEGIN_TYPE   '<'
        #define END_TYPE     '>'
        #define BEGIN_PORT   '|'
        #define END_PORT     '|'

namespace Glip
{
    namespace CorePipeline
    {
        // Prototypes
            class ComponentLayout;
        // Object
            class ObjectName
            {
                private :
                    // Data
                        std::string name; // Mandatory
                        std::string type; // Optionnal
                        int portID;       // Optionnal
                    // Tools
                        static void removeQualifier(std::string& str, const std::string& beginQual, const std::string& endQual);
                protected :
                    // Tools
                        ObjectName(const std::string& name, const std::string& type); // For a  Filter or a Pipeline with name and type
                        ObjectName(const std::string& name, int port);                // For a port with a name
                        void checkName(void);
                        void checkType(void);
                    // Friends
                        friend class ComponentLayout;
                public :
                    // Tools
                        ObjectName(const ObjectName&); // As a copy
                        void               setName(const std::string&);
                        const std::string& getName(void) const;
                        std::string        getNameExtended(void) const;
                        const std::string& getType(void) const;
                    // Static Tools
                        static std::vector<std::string> parse(const std::string& field);
            };

        // Tools
            template<typename T>
            T& getInstanceByName(const std::string& str, const std::vector<T>& lst)
            {
                if(lst.empty())
                    throw Exception("getInstanceByName (Vector) - Vector of items is empty", __FILE__, __LINE__);

                for(typename std::vector<T>::const_iterator it=lst.begin(); it!=lst.end(); it++)
                    if((*it).getName()==str)
                        return (*it);

                throw Exception("getInstanceByName (Vector) - No Object named : " + str, __FILE__, __LINE__);
            }

            template<typename T>
            T& getInstanceByNamePtr(const std::string& str, const std::vector<T*>& lst)
            {
                if(lst.empty())
                    throw Exception("getInstanceByNamePtr (Vector) - Vector of items is empty", __FILE__, __LINE__);

                for(typename std::vector<T*>::const_iterator it=lst.begin(); it!=lst.end(); it++)
                    if((*it)->getName()==str)
                        return (*it);

                throw Exception("getInstanceByName (Vector) - No Object named : " + str, __FILE__, __LINE__);
            }

            template<typename T>
            int getIndexByName(const std::string& str, const std::vector<T>& lst)
            {
                if(lst.empty())
                    throw Exception("getIndexByNamePtr (Vector) - Vector of items is empty", __FILE__, __LINE__);

                int k = 0;
                for(typename std::vector<T>::const_iterator it=lst.begin(); it!=lst.end(); it++)
                {
                    if((*it).getName()==str)
                        return k;
                    else
                        k++;
                }

                throw Exception("getIndexByName (Vector) - No Object named : " + str, __FILE__, __LINE__);
            }

            template<typename T>
            int getIndexByNamePtr(const std::string& str, const std::vector<T*>& lst)
            {
                if(lst.empty())
                    throw Exception("getIndexByNamePtr (Vector) - Vector of items is empty", __FILE__, __LINE__);

                int k = 0;
                for(typename std::vector<T*>::const_iterator it=lst.begin(); it!=lst.end(); it++)
                {
                    if((*it)->getName()==str)
                        return k;
                    else
                        k++;
                }

                throw Exception("getIndexByNamePtr (Vector) - No Object named : " + str, __FILE__, __LINE__);
            }

            int getIndexByNameFct(const std::string& str, const int ln, ObjectName& (*f)(int, const void*), const void* obj);
    }
}

#endif