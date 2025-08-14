<?php
class phpMQTT {
    private $socket;
    public $address;
    public $port;
    public $clientid;
    public $keepalive;
    public $cafile;
    public $debug = false;

    function __construct($address, $port, $clientid) {
        $this->address = $address;
        $this->port = $port;
        $this->clientid = $clientid;
    }

    function connect() {
        $this->socket = fsockopen($this->address, $this->port, $errno, $errstr, 60);
        if (!$this->socket) {
            return false;
        }
        return true;
    }

    function publish($topic, $msg) {
        $msg = $msg."\n";
        fwrite($this->socket, "PUBLISH {$topic} {$msg}");
        return true;
    }

    function close() {
        fclose($this->socket);
    }
}
