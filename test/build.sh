#!/bin/env sh

rule="cc -Wall -Werror -Wextra -g main.c ../scfp.a"

$rule
echo $rule