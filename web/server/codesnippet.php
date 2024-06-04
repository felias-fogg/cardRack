// Enter this code in the window of the CodeSnippet Wordpress plugin
function cards_func( $atts ) {
	$a = shortcode_atts( array(
		'url' => 'https://example.org',	
		'default' => 'no info',	
		'singular' => '%d',
		'plural' => '%d',
	), $atts );	
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

add_shortcode("cards", "cards_func");