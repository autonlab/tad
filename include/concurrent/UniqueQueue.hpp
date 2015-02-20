/*
   Date:         February 19, 2015
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __Concurrent_Unique_Queue_hpp__
#define __Concurrent_Unique_Queue_hpp__

#include "MutexLocker.hpp"
#include <queue>
#include <set>

namespace al { namespace concurrent
{
    /*!
     * This is a generic thread-safe wrapper for a STL queue which uses
     * a mutex to isolate read and write events. It also keeps elements in
     * a set so only unique elements are added.
     */
    template <class T>
    class UniqueQueue
    {
        public:
            /*!
             * Push an element into the queue.
             * @param element The element to insert into the queue.
             */
            inline void push( T element )
            {
                MutexLocker locker(mutex);
                if (s.find(element) == s.end())
                {
                    q.push(element);
                    s.insert(element);
                }
            }

            /*!
             * Push many elements into the queue at once.
             * @param elements The elements to insert into the queue.
             */
            inline void push( std::vector<T> elements )
            {
                MutexLocker locker(mutex);
                for (int e = 0; e < elements.size(); ++e)
                {
                    if (s.find(elements[e]) == s.end())
                    {
                        q.push(elements[e]);
                        s.insert(elements[e]);
                    }
                }
            }

            /*!
             * Pop the next element from the queue. This returns the element,
             * removes it from the queue, and lets you know if it was unsuccessful
             * (i.e. if you tried to pop when there were no elements).
             * @param [out] element The element that will be written to with the
             *          popped value.
             * @param release_from_set If true, this will also remove the popped
             *          element from the set, allowing it to be reinserted into
             *          the queue. You may want this to be set to false to allow
             *          popping an element and preventing it from being reinserted
             *          until finished processing, after which you'd use release.
             * @return True if an element existed and was popped, false if the
             *          queue was empty.
             * @see release
             */
            inline bool pop( T & element, const bool release_from_set = true )
            {
                MutexLocker locker(mutex);
                if (q.empty()) return false;
                else
                {
                    element = q.front();
                    q.pop();
                    if (release_from_set) s.erase(s.find(element));
                    return true;
                }
            }

            /*! Pop all elements from the queue.
             * @param [out] elements The vector where the elements are stored.
             * @return The number of elements returned.
             */
            inline int pop( std::vector<T> & elements )
            {
                MutexLocker locker(mutex);
                if (q.empty()) return 0;
                else
                {
                    elements.resize(q.size());
                    for (int e = 0; e < elements.size(); ++e)
                    {
                        elements[e] = q.front();
                        q.pop();
                    }
                    s.clear();
                    return elements.size();
                }
            }

            /*!
             * Releases an element from the queue's set. This means the element
             * can be reinserted into the queue, regardless of whether or not it's
             * already there. This is usually used in combination with pop using
             * release_from_set = false to provide time to process the element.
             * @param element The element to release.
             */
            inline void release( const T element ) { s.erase(s.find(element)); }

            /*!
             * @return Number of elements in queue.
             */
            inline typename std::queue<T>::size_type size( void )
            {
                MutexLocker locker(mutex);
                return q.size();
            }

            /*!
             * @return True if the element is already in the queue, false otherwise.
             */
            inline bool is_in_queue( const T element )
            {
                MutexLocker locker(mutex);
                return s.find(element) != s.end();
            }

        private:
            std::queue<T> q;
            std::set<T> s;
            Mutex mutex;
    };
} }

#endif
