package main

import (
	"fmt"
	"github.com/stretchr/testify/assert"
	"testing"
)

func Test_ParsePBMP(t *testing.T) {
	pbmp, err := ParsePBMP("10")
	assert.Nil(t, err)
	fmt.Println(pbmp)

	pbmp, err = ParsePBMP("1,2,3,4")
	assert.Nil(t, err)
	fmt.Println(pbmp)

	pbmp, err = ParsePBMP("1-10,20")
	assert.Nil(t, err)
	fmt.Println(pbmp)
}
