#!/bin/bash
BASE_URL="https://localhost:4433"

get_resource () {
	local endpoint="$1"
	local expected_response="$2"
	actual_response="$(curl -k -i -H "Connection: close" -X GET $BASE_URL/$endpoint 2>/dev/null)"
	assert_response "$expected_response" "$actual_response"
	return $?
}

head_resource () {
	local endpoint="$1"
	local expected_response="$2"
	actual_response="$(curl -k -H "Connection: close" --head $BASE_URL/$endpoint 2>/dev/null)"
	assert_response "$expected_response" "$actual_response"
	return $?
}

post_resource () {
	local endpoint="$1"
	local expected_response="$2"
	actual_response="$(curl -k -i -X POST -H "Connection: close" -H "Content-Type: text/plain" -d $'Some example text\r\n' $BASE_URL/$endpoint 2>/dev/null)"
	assert_response "$expected_response" "$actual_response"
	return $?
}

delete_resource () {
	local endpoint="$1"
	local expected_response="$2"
	actual_response="$(curl -k -i -X  DELETE -H "Connection: close" $BASE_URL/$endpoint 2>/dev/null)"
	assert_response "$expected_response" "$actual_response"
	return $?
}

put_resource () {
	local endpoint="$1"
	local expected_response="$2"
	actual_response="$(curl -k -i -X -H "Connection: close" PUT -d $'Some example text\r\n' $BASE_URL/$endpoint 2>/dev/null)"
	assert_response "$expected_response" "$actual_response"
	return $?
}

assert_response () {
	local expected_response="$1"
	local actual_response="$2"	
	if [ "$actual_response" = "$expected_response" ]; then
		echo "Success! Response matches the expected response."
		return 1
	else
		echo "Error! Response does not match the expected response."
		echo -e "Expected response:\n$expected_response"
		echo -e "Actual response:\n$actual_response"
		return 0
	fi
}

success=0

response="$(cat expected_responses/get_response.txt)"
echo "Asserting get..."
get_resource "test.txt" "$response"
((success+=$?))

response="$(cat expected_responses/head_response.txt)"
echo "Asserting head..."
head_resource "test.txt" "$response"
((success+=$?))

response="$(cat expected_responses/delete_response.txt)"
echo "Asserting delete..."
delete_resource "test.txt" "$response"
((success+=$?))

response="$(cat expected_responses/post_response.txt)"
echo "Asserting post..."
post_resource "test.txt" "$response"
((success+=$?))

response="$(cat expected_responses/put_response.txt)"
echo "Asserting put..."
put_resource "test.txt" "$response"
((success+=$?))

echo "$success out of 5 tests successful"
