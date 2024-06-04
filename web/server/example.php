<!DOCTYPE html>
<html>
    <head>
        <title>cardRack-Test</title>
    </head>
    <body>
      <?php
        function read_card_num($noinfo = "no info") {
                $a['url'] = "https://iot-rack.de/data/example.txt";
		$a['default'] = "no info";
		$a['singular'] = "%d";
		$a['plural'] = "%d";
                $headers = @get_headers($a['url']);
		if($headers && strpos( $headers[0], '200')) { 
		  $data = explode("\n",file_get_contents($a['url']));
		} else { 
		  return $a['default'];
		} 

		if (time()-$data[1] > 3600+300 ) { 
		  return $a['default'];
		} else {
		  if ($data[0] == 1)  {
		    return sprintf($a['singular'],$data[0]);		
		  } else {
		    return sprintf($a['plural'],$data[0]);
		  }
		}
         }
      ?>
    <center>
       <br>
       <br>
       <b>Available cards: <?php echo read_card_num("No information available") ?></b>
    </center>
    </body>
</html>