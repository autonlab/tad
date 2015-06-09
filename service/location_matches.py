#!/usr/bin/python

def location_matches( string, seq ):
    if isinstance(string, str): fields = string.split(';')
    if isinstance(seq, str): seq = [seq]
    for f, s in zip(fields, seq):
        if s and (f != s): return False
    return True
