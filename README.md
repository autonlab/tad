Temporal Anomaly Detector (TAD)
===============================

Version 1.2 
Kyle Miller - mille856@cmu.edu  
Anthony Wertz - awertz@cmu.edu  
Carnegie Mellon University - Auton Lab

**NOTE** This readme is valid for the version of the software specified at the
top of this readme, not necessarily for the current repository revision!

Description
-----------

This is a service developed for the DARPA MEMEX program that performs temporal
anomaly detection on advertisments.

Building
--------

The service has two main components that can be built. First, there is the service
itself which can run stand-alone. Second, a docker image can be built (and there
is a ready-made image on Dockerhub) to contain the service and all required services.

There is nothing to build for the python service. Just use the `start_worker` and
`start_service` scripts. These scripts assume Celery is installed and a RabbitMQ
server is running with default guest:guest credentials.

To build the docker container, move to the `samples/docker` directory and run
the `./build-docker` script. Alternatively, you can pull the already built image
from dockerhub with `docker pull autonlab/tad`.

Starting the Python service directly
------------------------------------

The python service will attempt to connect to Celery and RabbitMQ on localhost. In
addition, the script also needs a configuration in order to access elastic search.
For elastic search, you can create a configuration file: see config/tad.cfg.example.
The file should be named tad.cfg and should be available in the "config" directory
relative to the working directory. For other settings, there are no script inputs,
so if you want to change these settings you'll need to modify the `tad.py` script.

The script requires some python packages: `numpy`, `fisher`, `flask`, `flask_restful`,
`celery` are required, all of which can also be installed via `pip`.

Running the service is achieved simply by

```
./start_worker &
./start_service
```

Starting the service using the Docker image
-------------------------------------------

By either building the docker image or by pulling it from dockerhub, you can
load everything pre-configured by simply running the container. The easiest
way to do this is using the script `samples/docker/run-docker`. In general
it should be provided with a log output directory and a configuration directory
like this:

```
./run-docker my/log/dir my/config/dir
```

After running this, the RESTful communication port will not be 5000 on the host
machine (that is the local port in the docker image but not the port on the
docker host). To find the docker port run something like

```
docker inspect autonlab-tad
```

or more specifically

```
docker inspect autonlab-tad | grep -n HostPort | awk '{print $3}' | sed 's/\"//g'
```

to grab the port number to use.

Using the service: Python client example
----------------------------------------

In `samples/client` there is a simple curl client implemented which, with
little or no error checking, will send a request and continuously monitor
the results until killed.

Using the service: REST API
---------------------------

The general approach to a query is

1. Send a request. A status message will be returned indicating that the
   request has been queued. Once a worker process is available it will
   handle the request. The task ID will be returned. This is a POST
   request to server:port/event-report
2. Periodically ping for a status update using the task ID provided in
   the reponse from step one. Either a status update will be returned or
   the results if finished. This is a GET request to server:port/event-report/task-id
3. If the results of that task are required again at some point in time,
   just query for the progress of the same task ID.

### POST event-report message

This message must be sent in order to create an event report. An event
report query is performed to compare ad volume over a specific period
of time with past ad volume, comparing two different regions. In other
words, the report will indicate whether ad volume of the target location
relative to the baseline location in the past is significantly different
than ad volume of the target location to the baseline location in the
period of interest. This is done using the Fisher exact test for
independence using the ad volumes in the reference and current time
periods for the target and baseline locations. The contingency table
looks like this:

            | reference | current
------------|:---------:|:------:
target      | #         | #
baseline    | #         | #

The service field of the header must indicate `CheapEventReport`. The body
contains the following fields.

Field               | Values (default)  | Explanation
--------------------|-------------------|------------
target-location     | string            | Target location(s)
baseline-location   | string            | Baseline location(s)
keylist             | string array      | Search keywords
analysis-start-date | date string       | Start date of analysis period in format %Y/%m/%d
analysis-end-date   | date string       | End date of analysis period
current-window      | int > 0 (7)       | Size (in days) of current window
reference-window    | int > 0 (91)      | Size (in days) of reference window
lag                 | int >= 0 (0)      | Lag (in days) of reference window  behind current window
tailed              | upper, lower, two | Tail for Fisher exact test
                    |    (upper)        |
data-source         | elasticsearch,    | Source of data to use
                    |   flatfile (es)   |
stratify            | bool (false)      | Whether or not to stratify events.
cluster-field       | string (phone)    | The database field on which to cluster (not used for flatfiles)

The target and baseline locations indicate the locations, separated by
commas, to be used for the target and baseline comparisons. For elastic search,
the locations should include city, state, and country separated by semicolons.
Keylist (which can be empty) is a set of keywords you wish to search for.
A result will be returned so long as it includes at least one of the keywords.
Analysis start and end date represent the period of time your interested
in analysing and are represented using the `strptime` format of `%Y/%m/%d`,
so for example "2015/05/14". 

The window sizes represent the windows to aggregate counts in for the
current time period and the reference period. For example, you may compare
the seven days leading up to a date of interest (current-window = 7) to the
previous three months (reference-window = 90). Use the lag parameter if you
want to create more separation between the windows. For example, maybe you
want to compare it to the same time last year. You could set the current
and reference windows to be the same size and the lag to be `365 - current-window`
days long.

Tailed determines what kind of p-value test to perform with the test. Given
the table configuration, a lower tail test will yield significant p-values
when there appears to be an increase in ad volume in the current time period
of interest in the target location relative to the baseline. An upper tailed
test will yield significant p-values when the ad volume in the current time
period in the target location seems to decrease. Two-tailed test will detect
both of these changes.

The data source allows selection of the elastic search or a flatfile to be used.

The `stratify` field determines whether or not the results will be stratified
(described later). The `cluster-field` indicates the elastic search field
on which entities are clustered.

### GET event-report message

This message will allow querying for progress on the task. The body contains
just one important field:

Field   | Values | Explanation
--------|--------|------------
task-id | string | ID number of the task

The task ID is returned in the response when an event report is requested.
The reponse to this message will include the status of the current task or,
if finished processing, the results of the query.

Handling query results
----------------------

When a query is run, a record is returned for each day within the analysis
time period. Each record is a multi-type array containing the following fields
(in order, accessed by index).

First is the date.

The second is a block of counts for the current time window in the target
and baseline. In this block, the first column and every second column after
that represent the counts in the target window. The second column and every
second column after that represent the counts in the reference window.
There are four such pairs representing the four ad stratifications: overall,
local, new to town, and first post. (**NOTE** If stratification is disabled,
there is only one pair: overall). Therefore, this block has eight fields
ordered as follows:

```
target (overall), reference (overall), target (local), reference (local), ...
```

There is another block structured identically, representing the counts
in the reference period.

Finally, there is a block of four p-values. They are ordered by the same
stratification as above: overall, local, new to town, and first. (Or only
overall in the case stratification is disabled.) So that looks like this:

```
p-value (overall), p-value (local), ...
```

Configuring input: flatfiles
----------------------------

A CSV text file may be used to provide a data source from which to query.
It must be named `records.csv` and reside in a `snapshot` directory relative
to the starting point of the TAD service. If launching the Python script
directly, the directory should be in your calling directory. If using the
docker container, the data directory must be sent while configuring the
service.

The CSV file contains nine columns of data:

Index   | Name      | Description
:------:|-----------|------------
0       | ID        | The record's ID number
1       | Location  | String representing the location of the ad (e.g. Colorado Springs)
2       | State     | String representing the U.S. state of the ad (e.g. Colorado)
3       | Date      | The date represented in %b/%d/%Y format, e.g. Mar/14/2015
4       | Age       | The age of the victim or UNSPECIFIED
5       | Size      | The size of the victim or UNSPECIFIED
6       | Phone     | The victim's phone number or a unique placeholder (e.g. p-ID#)
7       | Cluster   | The cluster ID of the ad
8       | Content   | The add keywords

Using the docker container, the snapshot directory is the second argument
to the `run_docker` script provided in the `samples` directory. That directory
is mapped (read-only) to `snapshot`.

Configuring input: Elastic Search
---------------------------------

For configuring elastic search on your system, refer to online documentation. The
elastic search documents should contain fields `city`, `state`, `country`, and
`posttime`. The connection information needs to be in a file `config/tad.cfg`
relative to the working directory from which you start the service. There is an
example of a configuration file in the repository which just needs some basic
host information and credentials.

For configuring elastic search on the docker image, you just need to supply the
configuration directory which is the second parameter of the `run_docker` script
which is mounted to `config`.

License
-------

The MIT License (MIT)

Copyright (c) 2015 Carnegie Mellon University Auton Lab

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
