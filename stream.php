<?php

php_stream * stream = php_stream_open_wrapper("http://www.php.net", "rb", REPORT_ERRORS, NULL);

if (stream) {
	while(!php_stream_eof(stream)) {
		char buf[1024];

		if (php_stream_gets(stream, buf, sizeof(buf))) {
			printf(buf);
		} else {
			break;
		}
	}
	php_stream_close(stream);
}
