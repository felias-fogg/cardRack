<!DOCTYPE html>
<html>
    <head>
        <title>cardRack-Test</title>
    </head>
    <body>
      <?php
        function read_card_num($noinfo = "no info") {
             $data = explode("\n",file_get_contents("data/schallstadt.txt"));
             if (time()-$data[1] > 2*3600+120 ) {
	       return $noinfo;
	     } else {
	       return $data[0];
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