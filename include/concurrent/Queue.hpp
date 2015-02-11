/*
   Date:         September 22, 2014
   Author(s):    Anthony Wertz
   Copyright (c) Carnegie Mellon University
*/
#ifndef __LIB_CONCURRENT_QUEUE_HPP__
#define __LIB_CONCURRENT_QUEUE_HPP__

#include "Mutex.hpp"
#include <queue>

namespace al { namespace concurrent
{
    /*!
     * This is a generic thread-safe wrapper for a STL queue which just uses
     * a mutex to isolate read and write events.
     */
    template <class T>
    class Queue
    {
        public:
            /*!
             * Push an element into the queue.
             * @param element The element to insert into the queue.
             */
            inline void push( T element )
            {
                MutexLocker locker(mutex);
                q.push(element);
            }

            /*!
             * Push many elements into the queue at once.
             * @param elements The elements to insert into the queue.
             */
            inline void push( std::vector<T> elements )
            {
                MutexLocker locker(mutex);
                for (int e = 0; e < elements.size(); ++e)
                    q.push(elements[e]);
            }

            /*!
             * Pop the next element from the queue. This returns the element,
             * removes it from the queue, and lets you know if it was unsuccessful
             * (i.e. if you tried to pop when there were no elements).
             * @param [out] element The element that will be written to with the
             *          popped value.
             * @return True if an element existed and was popped, false if the
             *          queue was empty.
             */
            inline bool pop( T & element )
            {
                MutexLocker locker(mutex);
                if (q.empty()) return false;
                else
                {
                    element = q.front();
                    q.pop();
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
                    return elements.size();
                }
            }

            /*!
             * @return Number of elements in queue.
             */
            inline typename std::queue<T>::size_type size( void ) const { return q.size(); }

        private:
            std::queue<T> q;
            Mutex mutex;
    };
} }

#endif
