<?php
	
    // If the user is not authentified, redirect to the login file
	if (!isset($_SERVER['PHP_AUTH_DIGEST'])) header("Location:index.php");

    // Define the main directory path
	define('ROOT_PATH', realpath(__DIR__)."/..");
	
	// Include the database manager file
	require_once(ROOT_PATH.'/lib/database.MySQL.php');

	// Get the file that contain the template of the page
	$page = file_get_contents('view/page.html');

	/*
     * Instancation of the database class
	*/

	$db = new db();

	/*
	 * Create an array with all permission type
	*/

	$permissions = array(
		'0' => 'New key',
		'1' => 'Allowed',
		'2' => 'Denied'
	);

	/*
     * Get the history in the database
	*/

	$history = $db->getHistory($_GET['begin'], $_GET['end']);

	/*
     * Create the history table
	*/

	// Get the template file that contain the history table
	$content = file_get_contents('view/partial/history/historyTable.html');
	// If access log(s) exist in the database, create the row(s)
	if(is_array($history)) $historyTableContent = showHistoryTable($history);
	// Append the rows in the content area of the history table template
	$content = str_replace("[[historyTableContent]]", $historyTableContent, $content);

	/*
	 * Insert the history table in the content area of the template
	*/

	$html = str_replace("[[content]]", $content, $page);

	/*
	 * Display the page
	*/

	echo $html;

	/* *******************
	 * Functions 		 *
	 * *******************/

    // Function that display the history table
	function showHistoryTable($history){
		//Set the permissions array and the database class object to global
		global $permissions, $db;
		// Will contain the history table rows
		$historyTable;
		// Foreach access log find in the database, create a row in the history table
		foreach ($history as $item) {
			$action = '';
			// Get the template that contain the history table row
			$line = file_get_contents('view/partial/history/historyTableRow.html');
			// Will set the color of the row
			$permissionColor = setColor($item['permission']);
			// Will determine if the key is new or not
			if($item['permission'] === '0') {
				// Find all user that have the key
				$result = $db->getUser($item['idkey']);
				// If no user have the key, it mean that it is a new key. Therefore, display a button that offer the possibility to add the key in the system 
				if (!is_array($result)) $action = str_replace("[[idkey]]", $item['idkey'], file_get_contents('view/partial/history/historyAddKey.html'));
			}
			// Place the history data in the different data area in the template
			$line = str_replace("[[firstname]]",$item['firstname'],$line);
			$line = str_replace("[[lastname]]",$item['lastname'],$line);
			$line = str_replace("[[idkey]]",$item['idkey'],$line);
			$line = str_replace("[[date]]",date("j M Y H:i:s", strtotime($item['date'])),$line);
			$line = str_replace("[[permission]]",$permissions[$item['permission']],$line);
			$line = str_replace("[[status]]",$permissionColor,$line);
			$line = str_replace("[[action]]",$action,$line);
			// Append the row to the table row
			$historyTable.= $line;
		}
		// Return all the row
		return $historyTable;
	}

	// Set the color of the row with the permission number
	function setColor($permission){
		// Will contain the color using a html class name
		$color;
		switch($permission){
			case '0':
				$color = 'warning';
				break;
			case '1':
				$color = 'success';
				break;
			case '2':
				$color = 'danger';
				break;
			default:
				break;

		}
		return $color;

	}

?>