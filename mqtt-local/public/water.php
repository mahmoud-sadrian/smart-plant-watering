<?php
require("phpMQTT.php");

$broker_ip = "192.168.1.6"; // IP سیستم شما که Mosquitto روش ران میشه
$topic = "watering/command";
$duration = isset($_GET['duration']) ? intval($_GET['duration']) : 5;

$mqtt = new phpMQTT($broker_ip, 1883, "phpClient".rand());
if ($mqtt->connect()) {
    $mqtt->publish($topic, strval($duration));
    $mqtt->close();
    echo "Command sent: Water for {$duration} seconds.";
} else {
    echo "Failed to connect to MQTT Broker.";
}
