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

func Clear(p unsafe.Pointer, buffer unsafe.Pointer) {
	C.free(buffer)
	C.UrlarbitratorClear(p)
}

func Insert(
	p unsafe.Pointer,
	txs []uint32,
	paths []string,
	reads []uint32,
	writes []uint32,
	composite []bool,
) (time.Duration, unsafe.Pointer) {
	begintime := time.Now()
	length := uint32(len(paths))
	pathLen := make([]uint32, length)

	for i, path := range paths {
		pathLen[i] = uint32(len(path))
	}
	buf := codec.Strings(paths).Flatten()
	cstr := C.CString(string(buf))

	// var buf bytes.Buffer
	// for i, p := range paths {
	// 	fmt.Fprintf(&buf, "%s", p)
	// }
	// cstr := C.CString(buf.String())

	elapsed := time.Since(begintime)

	C.UrlarbitratorInsert(
		p,
		(*C.char)(unsafe.Pointer(&txs[0])),
		(*C.char)(unsafe.Pointer(nil)),
		(*C.char)(unsafe.Pointer(cstr)),
		(*C.char)(unsafe.Pointer(&reads[0])),
		(*C.char)(unsafe.Pointer(&writes[0])),
		(*C.char)(unsafe.Pointer(&composite[0])),
		(*C.char)(unsafe.Pointer(&pathLen[0])),
		C.uint32_t(length),
	)
	return elapsed, unsafe.Pointer(cstr)
}

func Detect(p unsafe.Pointer, whitelist []uint32) ([]uint32, []uint32, []bool) {
	length := len(whitelist)
	txs := make([]uint32, length)
	groups := make([]uint32, length)
	flags := make([]bool, length)
	count := uint32(0)
	msg := make([]uint8, 4096) // 4K message buffer.

	C.UrlarbitratorDetect(
		p,
		(*C.char)(unsafe.Pointer(&whitelist[0])),
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
