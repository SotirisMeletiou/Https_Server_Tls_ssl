This file contains some general instructions on how to use the tls_server.
You can run make command in the same directory of the Makefile to create the tls_server executable 
and then run with ./tls_server
The tls server supports requests from postman , browsers(mozilla) and the command curl
The terminal shows both requests and responses for debugging purposes 
Preferable use closed connection because with the keep alive it waits for another request and stays there.
Place your key.pem and cert.pem inside the folder "keys"
---------------------------------------------------------------------------------------------------
That executable reads some configuration parameters from the config.txt
config.txt contains the following parameters:
THREADS specifies the number of worker threads for handling the http requests 
(the default value is 40)
PORT specifies the port number on which the https server will bind it's listening port to 
(default value is 4433)
HOME specifies the directory which contains all the resources the server can serve to 
the client (default is ./httphome)
---------------------------------------------------------------------------------------------------
The directory also contains some tests using bash scripts. The two tests are:
1) unit_testing.sh
Run with ./unit_testing.sh
This will attempt to perform 5 basic HTTPS requests using curl (GET, HEAD, POST, DELETE, PUT 
which is not implemented) receive some responses and compare them with the files in the 
expected_responses directory.
2) parallel_execution.sh
Run with ./parallel_execution.sh <number_of_times>
This will create a bunch of background processes using & that will each run the unit_testing.sh 
script allowing us to stress test the server and monitor parallel request handling.
---------------------------------------------------------------------------------------------------
We have also used the following bash command to monitor the number of open file descriptors
to monitor how our https server handles multiple sockets.
watch -n 1 "ls -l /proc/$(ps -e | grep tls_server | awk '{print $1}')/fd/"



