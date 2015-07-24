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
                'lte' : end.strftime('%Y-%m-%d')
            }
        }
    }

    # Build queries for the filters.
    l_queries = []
    for key in filters:
        l_queries.append({'match': {key: filters[key]}})

    # Define aggregation.
    aggs = { 'counts': { 'date_histogram': {
        'field': time_field,
        'interval': 'day',
        'min_doc_count': 0,
        'extended_bounds': {
            'min': start.strftime('%Y-%m-%d'),
            'max': end.strftime('%Y-%m-%d')
        }
    }}}

    # Now put it all together.
    final_query = \
    {
        'size': 0,
        'query': { 'bool': { 'must': [kl_query, range_query] + l_queries } },
        'aggs': aggs
    }

    # Return a single query with aggregation.
    return final_query

if __name__ == '__main__':
    import json

    q = es_query_generator(
            dt.datetime.strptime('2012-07-28', '%Y-%m-%d'),
            dt.datetime.strptime('2015-07-24', '%Y-%m-%d'),
            {'Location': 'MINOT_NORTH_DAKOTA'}, time_field='Date')
    print(json.dumps(q, indent=4, separators=(',', ': ')))
