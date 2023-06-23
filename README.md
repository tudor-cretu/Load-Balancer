<h5 align = center> Copyright Cretu Mihnea Tudor 315CAa 2022 - 2023 </h5> 
<h5 align = center> 2nd Homework for Data Structures and Algorithms, Year I - Faculty of Automation Control and Computer Science, Polytechnic University of Bucharest </h5>

<h1 align = center> Load Balancer </h1>
<br>
<h2> 1) Introduction </h2>
<br>
<p align = left> This program implements a Load Balancer which makes the connection between clients and servers using Consistent Hashing method. <i> Consistent Hashing is a distributed hashing scheme that operates independently of the number of servers or objects in a distributed hash table. It powers many high-traffic dynamic websites and web applications.</i> (according to <a href = https://www.toptal.com/big-data/consistent-hashing> Toptotal </a>)
<br>
<br>

<h2> 2) Usage </h2> 
<br>
<li> <b>add_server [id]</b> - adds a server with the given id in the on the load balancer and redirectes </li>
<li> <b>store [key] [value]</b> - stores the given key-value pair on the server with the corresponding hash </li>
<li> <b>retrieve [key]</b> - retrieves the value of the given key from the server with the corresponding hash </li>
<li> <b>remove_server [id]</b> - removes the server with the given id from the load balancer</li>
<br>

<h2> 3) Examples </h2>
<br>
<p> <li> <b>Input:</b> </p>
<p>-->add_server 0 </p>
<p>-->add_server 1 </p>
<p>-->add_server 2 </p>
<p>-->store "c674390f9" "Keyboard" </p>
<p>-->store "a3529213e15" "Headphones" </p>
<p>-->store "5a51719f5ec" "Router" </p>
<p>-->store "2fe5f9f583" "Laptop" </p>
<p>-->retrieve "c674390f9" </p>
<p>-->retrieve "2fe5f9f583" </p>
<p>-->remove_server 2 </p>
<p>-->retrieve "2fe5f9f583" </p>
<br>
<p> <li> <b>Output:</b> </p>
<p>Stored Keyboard on server 0. </p>
<p>Stored Headphones on server 1. </p>
<p>Stored Router on server 2. </p>
<p>Stored Laptop on server 2. </p>
<p>Retrieved Keyboard from server 0. </p>
<p>Retrieved Laptop from server 2. </p>
<p>Retrieved Laptop from server 1. </p>
<br> 

<h2> 4) Architecture </h2>
<br> 
<p> The program is divided into 4 main parts: </p>
<ul>
<li> <b>main.c</b> - contains the main function and the functions for parsing the input and calling the functions from the other files </li>
<li> <b>hashtable.c</b> - contains the functions for creating and managing the hash table </li>
<li> <b>server.c</b> - contains the functions for managing the servers, with each server being a hashtable </li>
<li> <b>load_balancer.c</b> - contains the functions for managing the load balancer, which makes the connection between the clients and the servers, redistributes keys and values between the servers if a new server is added or removed </li>
</ul>
<p> Each file has a header file with the declarations of the functions and the structures used in the respective file. </p>
<br> 
<br>

<p><b> ||| For more information regarding the functions purposes and their arguments, check the comments in the header(.h) files. </b></p>
<p><b> ||| For more information regarding the implemenation of each function, please check the comments in .c files. </b></p>