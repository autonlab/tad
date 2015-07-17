#!/usr/bin/python

import datetime as dt

def es_query_generator( start, end, filters, keylist = None, time_field = None ):
    if not isinstance(time_field, str):
        raise Exception('time_field must be a string!')

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
        'range': {
            time_field: {
                'gte': start.strftime('%Y-%m-%d'),
                'lt' : end.strftime('%Y-%m-%d')
            }
        }
    }

    # Build queries for the filters.
    l_queries = []
    for key in filters:
        l_queries.append({'match': {key: filters[key]}})

    # Now put it all together.
    final_query = \
    {
        'query': { 'bool': { 'must': [kl_query, range_query] + l_queries } }
    }

    # Return a query for each day.
    this_day = start
    next_day = start
    print(final_query)
    while True:
        next_day += dt.timedelta(days = 1)
        if this_day > end: break
        final_query['query']['bool']['must'][1]['range'][time_field]['gte'] = \
                this_day.strftime('%Y-%m-%d')
        final_query['query']['bool']['must'][1]['range'][time_field]['lt'] = \
                next_day.strftime('%Y-%m-%d')
        yield final_query
        this_day += dt.timedelta(days = 1)

if __name__ == '__main__':
    import json

    for q in es_query_generator(
            dt.datetime.strptime('07/15/2015', '%m/%d/%Y'),
            dt.datetime.strptime('07/17/2015', '%m/%d/%Y'),
            {'stock': 'ACOL'}, time_field='dt'):
        print(json.dumps(q, indent=2, separators=(',', ': ')))
