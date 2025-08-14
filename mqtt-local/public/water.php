<?php
require("phpMQTT.php");

$server = "192.168.1.10"; // آی‌پی سیستم شما که Mosquitto روش نصبه
$port = 1883;
$username = ""; 
$password = "";
$client_id = "PHPClient" . rand();

// گرفتن مدت زمان از فرم
$duration = isset($_POST['duration']) ? intval($_POST['duration']) : 5;

$mqtt = new phpMQTT($server, $port, $client_id);

if ($mqtt->connect(true, NULL, $username, $password)) {
    $mqtt->publish("plant/water", strval($duration), 0);
    $mqtt->close();
    echo "Watering for {$duration} seconds";
} else {
    echo "Failed to connect to MQTT broker";
}
?>
