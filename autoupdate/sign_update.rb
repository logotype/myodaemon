#!/usr/bin/ruby
puts `cp "../build/myodaemon.zip" . ; openssl dgst -sha1 -binary < "myodaemon.zip" | openssl dgst -dss1 -sign "../../dsa_priv.pem" | openssl enc -base64`