package wrapper

/*
#cgo CFLAGS: -I../arbitrator
#cgo LDFLAGS: -L./ -lurlarbitrator
#include "external.h"
#include <stdlib.h>
*/
import "C"

import (
	"fmt"
	"time"
	"unsafe"

	"github.com/HPISTechnologies/common-lib/codec"
)

func Start() unsafe.Pointer {
	return C.Start()
}

func Clear(p unsafe.Pointer, buffer unsafe.Pointer) {
	C.free(buffer)
	C.Clear(p)
}

func Insert(
	p unsafe.Pointer,
	txs []uint32,
	paths []string,
	reads []uint32,
	writes []uint32,
	addOrDelete []bool,
	composite []bool,
) (unsafe.Pointer, time.Duration) {
	length := uint(len(paths))
	pathLen := make([]uint32, length)
	t0 := time.Now()
	for i, p := range paths {
		pathLen[i] = uint32(len(p))
	}

	buf := codec.Strings(paths).Flatten()
	cstr := C.CString(string(buf))
	elapsed := time.Now().Sub(t0)

	C.Insert(
		p,
		(*C.char)(unsafe.Pointer(&txs[0])),
		(*C.char)(unsafe.Pointer(nil)),
		(*C.char)(unsafe.Pointer(cstr)),
		(*C.char)(unsafe.Pointer(&reads[0])),
		(*C.char)(unsafe.Pointer(&writes[0])),
		(*C.char)(unsafe.Pointer(&addOrDelete[0])),
		(*C.char)(unsafe.Pointer(&composite[0])),
		(*C.char)(unsafe.Pointer(&pathLen[0])),
		*(*C.uint)(unsafe.Pointer(&length)),
	)
	return unsafe.Pointer(cstr), elapsed
}

func Detect(p unsafe.Pointer, length uint32) ([]uint32, []uint32, []bool) {
	txs := make([]uint32, length)
	groups := make([]uint32, length)
	flags := make([]bool, length)
	count := uint32(0)
	msg := make([]uint8, 4096) // 4K message buffer.

	C.Detect(
		p,
		(*C.char)(unsafe.Pointer(&txs[0])),
		(*C.char)(unsafe.Pointer(&groups[0])),
		(*C.char)(unsafe.Pointer(&flags[0])),
		(*C.char)(unsafe.Pointer(&count)),
		(*C.char)(unsafe.Pointer(&msg[0])),
	)
	return txs[:count], groups[:count], flags[:count]
}

func ExportTxs(p unsafe.Pointer) ([]uint32, []uint32) {
	length := C.GetConflictTxTotal(p)
	if length == 0 {
		return []uint32{}, []uint32{}
	}
	left := make([]uint32, length)
	right := make([]uint32, length)
	count := uint32(0)

	C.ExportTxs(
		p,
		(*C.char)(unsafe.Pointer(&left[0])),
		(*C.char)(unsafe.Pointer(&right[0])),
		(*C.char)(unsafe.Pointer(&count)),
	)
	if uint32(length) != count {
		panic(fmt.Sprintf("wrong length, got %d, want %d", count, length))
	}
	return left, right
}
