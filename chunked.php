<?php

public someFunctionInsideController()
{
    $response = new Response();
    $response->headers->set('Content-Encoding', 'chunked');
    $response->headers->set('Transfer-Encoding', 'chunked');
    $response->headers->set('Content-Type', 'text/html');
    $response->headers->set('Connection', 'keep-alive');
    $response->sendHeaders();
    flush();
    ob_flush();
    for ($i = 0; $i < 1000; $i++) {
        $this->dump_chunk('Sending data chunk ' . ($i+1) . ' of 1000 <br />');
        flush();
        ob_flush();
        sleep(2);
    }
    return new Response();
}
//...
//I found this on some forum / article and seems to be useful
//Also I would like to give some credits to the author, but I don't really know where I dig this:
private function dump_chunk($chunk) {
    echo sprintf("%x\r\n", strlen($chunk));
    echo $chunk;
    echo "\r\n";
}
