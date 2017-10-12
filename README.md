# cnitize

Sanitize untrusted user input for inclusion in an HTML, XML or TSV document.

## Streaming use

cnitize uses block buffering for performance reason. If you want to pass it
a post and read the result immediately, you should make it use line buffering
with a command such as stdbuf.
