# docker-hive-cli

Docker files to generate a Hive CLI to Hadoop cluster. The output is 
a docker image, which can be posted to Docker Hub. It can be configured 
to run using one of two configurations: memex or darpa 

## Building the Hadoop/Hive CLI Docker Image 

Run in directory containing Dockerfile to create new Docker image. Change the 
parker20121/docker-hive-cli tag to fit your own Docker profile.

	sudo docker build --rm=true -t="parker20121/docker-hive-cli:v1" .

Once completed, run the new container from new parker20121/docker-hive-cli image.
Also link the host local directory, /home/docker/docker-hive-cli, to the 
container's directory, /tmp/config.

	sudo docker run -v /home/docker/docker-hive-cli:/tmp/config -t -i parker20121/docker-hive-cli:v1 /bin/bash

Once the image has started, copy the configuration files by running the copy-files.sh
script contained in the /tmp/config directory. From /tmp/config run:

	./copy-files.sh [CONFIGURATION]

where configuration is currently one of the following choices:

	memex - DARPA MEMEX Hadoop cluster configuration
	
	xdata - DARPA XDATA Hadoop cluster configuration

	[custom] - A subdirectory that contains your own Hadoop/Hive configuration files.

