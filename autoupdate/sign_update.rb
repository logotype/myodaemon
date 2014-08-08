#!/usr/bin/ruby
puts `openssl dgst -sha1 -binary < "../build/myodaemon.zip" | openssl dgst -dss1 -sign "../../dsa_priv.pem" | openssl enc -base64`