<?php
/*
    Simple PHP MQTT Class for Publish/Subscribe
    Author: https://github.com/bluerhinos/phpMQTT
*/

class phpMQTT {
    private $socket;
    private $msgid = 1;
    public $keepalive = 10;
    public $timesinceping;
    public $topics = [];
    public $debug = false;
    public $address;
    public $port;
    public $clientid;
    public $will;
    public $username;
    public $password;

    function __construct($address, $port, $clientid) {
        $this->address = $address;
        $this->port = $port;
        $this->clientid = $clientid;
    }

    function connect($clean = true, $will = NULL, $username = NULL, $password = NULL) {
        $this->will = $will;
        $this->username = $username;
        $this->password = $password;

        $this->socket = @fsockopen($this->address, $this->port, $errno, $errstr, 60);
        if (!$this->socket) {
            if ($this->debug) echo "Failed to connect: $errno, $errstr\n";
            return false;
        }

        stream_set_timeout($this->socket, 5);
        $i = 0;
        $buffer = "";

        $buffer .= chr(0x00);
        $buffer .= chr(0x04);
        $buffer .= "MQTT";
        $buffer .= chr(0x04); // Protocol Level = 4
        $var = 0;
        if ($clean) $var += 2;
        if ($will) $var += 4 + ($will['qos'] << 3) + ($will['retain'] << 5);
        if ($username) $var += 128;
        if ($password) $var += 64;

        $buffer .= chr($var);
        $buffer .= chr($this->keepalive >> 8);
        $buffer .= chr($this->keepalive & 0xff);
        $buffer .= $this->strwritestring($this->clientid);
        if ($will) {
            $buffer .= $this->strwritestring($will['topic']);
            $buffer .= $this->strwritestring($will['content']);
        }
        if ($username) $buffer .= $this->strwritestring($username);
        if ($password) $buffer .= $this->strwritestring($password);

        $head = "CONNECT";
        $head = chr(0x10);
        $this->write($head, $buffer);
        $string = $this->read(4);
        $cmd = ord($string[0]);
        $code = ord($string[3]);
        if ($cmd == 0x20 && $code == 0) {
            $this->timesinceping = time();
            return true;
        }
        return false;
    }

    function publish($topic, $content, $qos = 0, $retain = 0) {
        $buffer = $this->strwritestring($topic);
        if ($qos) {
            $id = $this->msgid++;
            $buffer .= chr($id >> 8);
            $buffer .= chr($id % 256);
        }
        $buffer .= $content;
        $cmd = 0x30;
        if ($qos) $cmd += $qos << 1;
        if ($retain) $cmd += 1;
        $this->write(chr($cmd), $buffer);
    }

    private function read($length = 8192) {
        $string = '';
        while (!feof($this->socket)) {
            $byte = fread($this->socket, 1);
            if ($byte === false) return false;
            $string .= $byte;
            if (strlen($string) >= $length) break;
        }
        return $string;
    }

    private function write($header, $buffer) {
        $remaining_length = strlen($buffer);
        $rl = "";
        do {
            $digit = $remaining_length % 128;
            $remaining_length = intval($remaining_length / 128);
            if ($remaining_length > 0) $digit = ($digit | 0x80);
            $rl .= chr($digit);
        } while ($remaining_length > 0);

        fwrite($this->socket, $header . $rl . $buffer);
    }

    private function strwritestring($string) {
        return chr(strlen($string) >> 8) . chr(strlen($string) & 0xff) . $string;
    }

    function close() {
        fclose($this->socket);
    }
}
?>
