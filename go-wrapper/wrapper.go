package wrapper

/*
#cgo CFLAGS: -I../arbitrator
#cgo LDFLAGS: -L/usr/local/lib -lurlarbitrator
#include "external.h"
#include <stdlib.h>
*/
import "C"

import (
	"time"
	"unsafe"

	"github.com/arcology-network/common-lib/codec"
)

const (
	MAX_ENTRIES = 65536
)

func Start() unsafe.Pointer {
	return C.UrlarbitratorStart()
}

func Clear(p unsafe.Pointer) {
	C.UrlarbitratorClear(p)
}

func Insert(
	p unsafe.Pointer,
	txs []uint32,
	paths []string,
	reads []uint32,
	writes []uint32,
	composite []bool,
) []time.Duration {
	tims := make([]time.Duration, 2)
	length := uint32(len(paths))
	pathLen := make([]uint32, length)

	t := time.Now()
	for i, path := range paths {
		pathLen[i] = uint32(len(path))
	}
	buf := codec.Strings(paths).Flatten()
	tims[0] = time.Since(t)

	t = time.Now()
	C.UrlarbitratorInsert(
		p,
		(*C.char)(unsafe.Pointer(&txs[0])),
		(*C.char)(unsafe.Pointer(&buf[0])),
		(*C.char)(unsafe.Pointer(&pathLen[0])),
		(*C.char)(unsafe.Pointer(&reads[0])),
		(*C.char)(unsafe.Pointer(&writes[0])),
		(*C.char)(unsafe.Pointer(&composite[0])),
		C.uint32_t(length),
	)
	tims[1] = time.Since(t)
	return tims
}

func DetectLegacy(p unsafe.Pointer, whitelist []uint32) ([]uint32, []uint32, []bool) {
	length := len(whitelist)
	txs := make([]uint32, length)
	groups := make([]uint32, length)
	flags := make([]bool, length)
	count := uint32(0)
	msg := make([]uint8, 4096) // 4K message buffer.
	groupIDLens := make([]uint32, length)
	for i := 0; i < length; i++ {
		groupIDLens[i] = 1
	}

	C.UrlarbitratorDetect(
		p,
		(*C.char)(unsafe.Pointer(&whitelist[0])),
		(*C.char)(unsafe.Pointer(&groupIDLens[0])),
		(C.uint32_t)(length),
		(*C.char)(unsafe.Pointer(&txs[0])),
		(*C.char)(unsafe.Pointer(&count)),
		(*C.char)(unsafe.Pointer(&msg[0])),
	)

	for i := 0; i < int(count); i++ {
		flags[i] = true
	}
	return txs[:count], groups[:count], flags[:count]
}

func Detect(p unsafe.Pointer, groups [][]uint32) ([]uint32, []uint32, []bool, []time.Duration, int) {
	tims := make([]time.Duration, 3)
	t := time.Now()
	groupIDCount := len(groups)
	groupIDLens := make([]uint32, groupIDCount)
	total := 0
	for i, group := range groups {
		length := len(group)
		groupIDLens[i] = uint32(length)
		total = total + length
	}
	groupIDs := make([]uint32, 0, total)
	for _, group := range groups {
		groupIDs = append(groupIDs, group...)
	}
	outTxBuf := make([]uint32, len(groupIDs))
	outCount := uint32(0)
	msgBuffer := make([]uint8, 4096)
	tims[0] = time.Since(t)
	t = time.Now()
	C.UrlarbitratorDetect(
		p,
		(*C.char)(unsafe.Pointer(&groupIDs[0])),
		(*C.char)(unsafe.Pointer(&groupIDLens[0])),
		(C.uint32_t)(groupIDCount),
		(*C.char)(unsafe.Pointer(&outTxBuf[0])),
		(*C.char)(unsafe.Pointer(&outCount)),
		(*C.char)(unsafe.Pointer(&msgBuffer[0])),
	)
	tims[1] = time.Since(t)
	t = time.Now()
	outGroups := make([]uint32, outCount)
	flags := make([]bool, outCount)
	for i := 0; i < int(outCount); i++ {
		flags[i] = true
	}
	tims[2] = time.Since(t)
	return outTxBuf[:outCount], outGroups, flags, tims, total
}

func ExportTxs(p unsafe.Pointer) ([]uint32, []uint32) {
	left := make([]uint32, MAX_ENTRIES)
	right := make([]uint32, MAX_ENTRIES)
	count := uint32(0)

	C.UrlarbitratorExportConflictPairs(
		p,
		(*C.char)(unsafe.Pointer(&left[0])),
		(*C.char)(unsafe.Pointer(&right[0])),
		(*C.char)(unsafe.Pointer(&count)),
	)
	return left[:count], right[:count]
}
