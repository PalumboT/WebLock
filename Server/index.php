<?php

	/*
	 * Copyright (C) 2014
	 * 	Timothée Palumbo (https://github.com/PalumboT)
	 * 
	 * Weblock project is a RFID lock connected that use a web server 
	 * for the authentication. This is the authentification service
	 * 
	*/

	/*
     * Define the main directory path
	*/

	define('ROOT_PATH', realpath(__DIR__));

	/*
     * Include the database manager class file
	*/

	require("lib/db.php");

	/*
	 * Initialisation of the database class
	*/
	
	$db = new db();

	/*
	 * Get the value of the key in the URL
	*/

	$key = $_GET["key"];

	/*
     * Get the user list in the database
	*/

	$user = $db->getUser($key)[0];

	/*
	 * If the key is assigned to an user
	*/
	
	if (is_array($user)) {
		// Log the access with the user data
		$db->addAccess($key, $user);
		// Return the http code 200 to the lock if the user have the permission to access, else http code 401
		$user['permission'] == 1 ? header("HTTP/1.1 200 Ok") : header("HTTP/1.1 401 Unauthorized");
		// Die the programme
		die();
	}
	
	/*
	 * Else if the key doesn't exist, log the access 
	 * and return the 401 http code
	*/

	// Log the access only with the key
	$db->addAccess($key);
	// Return the hhtp code 401 to the lock
	header("HTTP/1.1 401 Unauthorized");

?>