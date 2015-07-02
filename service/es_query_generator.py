#!/usr/bin/python

import datetime as dt

def es_query_generator( start, end, filters, keylist = None ):
    # Build query for keylist, if any.
    kl_query = None
    if keylist == None: keylist = ''
    if isinstance(keylist, (list, tuple)): keylist = ' '.join(keylist)
    if len(keylist) > 0:
        kl_query = \
        {
            'query_string':
            {
                'query': keylist,
                'default_operator': 'OR'
            }
        }

    # Build query for date range.
    range_query = \
    {
        'posttime': {
            'gte': start.strftime('%Y-%m-%d'),
            'lt' : end.strftime('%Y-%m-%d')
        }
    }

    # Build queries for the locations.
    l_queries = []
    for key in filters:
        l_queries.append({'term': {key: filters[key]}})

    # Build filter.
    filter_query = { 'and': [ {'and': l_queries}, {'range': range_query} ] }

    # Now put it all together.
    final_query = \
    {
        'query': { 'filtered': { 'query': kl_query, 'filter': filter_query } }
    }

    # Return a query for each day.
    this_day = start
    next_day = start
    while True:
        next_day += dt.timedelta(days = 1)
        if this_day > end: break
        final_query['query']['filtered']['filter']['and'][1]['range']['posttime']['gte'] = \
                this_day.strftime('%Y-%m-%d')
        final_query['query']['filtered']['filter']['and'][1]['range']['posttime']['lt'] = \
                next_day.strftime('%Y-%m-%d')
        yield final_query
        this_day += dt.timedelta(days = 1)

if __name__ == '__main__':
    import json

    for q in es_query_generator(
            dt.datetime.strptime('09/23/1986', '%m/%d/%Y'),
            dt.datetime.strptime('09/25/1986', '%m/%d/%Y'),
            {'eyes': 'green', 'height': 'almost 6 ft', 'weight': 'how rude'},
            ['i', 'am', 'a', 'red', 'butt', 'monkey']):
        print(json.dumps(q, indent=2, separators=(',', ': ')))
