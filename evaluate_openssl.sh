#!/bin/sh
$PWD/enclave/openssl/apps/openssl speed > native_openssl_speed_output.txt
$PWD/openssl speed > porpoise_openssl_speed_output.txt
