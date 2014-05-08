<?php

	/**
	 * Copyright (C) 2014
	 * 	Timothée Palumbo (https://github.com/PalumboT)
	 * 
	 * Weblock project is a lock connected that use a web server 
	 * for the authentication. This is the authentification service
	 * 
	*/

	/*
     * Define the main directory path
	*/

	define('ROOT_PATH', realpath(__DIR__));

	/*
     * Include the database manager file
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

	$users = $db->getUsers();

	/*
	 * If the key exist in the users array, return the 200 http code.
	 * the http code will be interpreted by the lock to open the door.
	*/

	foreach($users as $user){
		if (in_array($key, $user, true)) {
			header("HTTP/1.1 200 Ok"); 
			die();
		}
	}
	
	/*
	 * Else if the key doesn't exist, return the 401 http code
	*/

	header("HTTP/1.1 401 Unauthorized");

?>