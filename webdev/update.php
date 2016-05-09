<?php 
$returnData = "";

foreach($_POST as $key => $val) {
  print $key.' => '.$val.'\r\n';
}

#$data = $_POST['data'];
#$fileName = $_POST['fileName'];
#$serverFile = time().$fileName;
#$returnData = $filename . ':' .  $serverFile ;
echo $returnData;

?>