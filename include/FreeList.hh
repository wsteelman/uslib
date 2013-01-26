
#ifndef FREE_LIST_HH_
#define FREE_LIST_HH_

#include <list>

namespace uslib
{

template <class T>
class FreeList
{
public:
   FreeList() : 
      m_size(0)
   {
      pthread_mutex_init(&m_mutex, NULL); 
   }

   ~FreeList()
   {

   }

   void replace(T *item)
   {
      pthread_mutex_lock(&m_mutex);
      m_free_list.push_back(item);
      m_size++;
      pthread_mutex_unlock(&m_mutex);
   }

   T *get()
   {
      pthread_mutex_lock(&m_mutex);
      if (m_free_list.empty())
      {
         pthread_mutex_unlock(&m_mutex);
         return NULL; 
      }
      T *item = m_free_list.front();
      m_free_list.pop_front();
      m_size--;
      pthread_mutex_unlock(&m_mutex);
      return item;
   }

   bool empty()
   {
      pthread_mutex_lock(&m_mutex);
      bool empty = m_size == 0;
      pthread_mutex_unlock(&m_mutex);
      return empty;
   }

   uint32 size()
   {
      return m_size;  
   }

private:
   int             m_size;
   std::list<T*>   m_free_list;
   pthread_mutex_t m_mutex;

}; // class FreeList
} // namespace uslib
#endif
