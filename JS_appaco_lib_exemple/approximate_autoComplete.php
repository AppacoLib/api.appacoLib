<?php
 
	//  the EOL pbm : if for example the file was mad on linux (EOL=\n) and then you go to windows, so PHP_EOL=\r\n so it's not working cause we have tow defrent value
	// $my_array = explode(PHP_EOL,file_get_contents('english.txt'));// get all words
	$my_array = preg_split ("/\r\n|\n|\r/",file_get_contents('english.txt'));// get all words
	
    $size_my_array = count($my_array);
 
 
    $results = array();
  
	for ($i = 0 ; $i < $size_my_array && count($results) < $_GET['SIZE_GROUP'] ; $i++) 
	{
		if (stripos($my_array[$i], $_GET['queryWord']) === 0) // if it's begin with the same prefix
		{ 
			array_push($results, $my_array[$i]);
		}
	}
 
	echo json_encode($results);
 
 /*
	$size_results = count($results) -1;
	echo "[";
	
    for ($i = 0 ; $i < $size_results ; $i++) 
	{
		echo "\"",$results[$i],"\",";
	}
	
	echo "\"",$results[$i],"\"]"; // the last one
	*/
 
?>