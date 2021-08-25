package wrapper_test

import (
	"testing"

	"github.com/arcology/urlarbitrator-engine/go-wrapper"
)

func TestWrapperBasic(t *testing.T) {
	engine := wrapper.Start()
	wrapper.Insert(
		engine,
		[]uint32{2, 2, 2, 2, 3, 3, 3, 3},
		[]string{
			"blcc://eth1.0/contractAddress/storage/containers/!/arrayID",
			"blcc://eth1.0/contractAddress/storage/containers/arrayID/#",
			"blcc://eth1.0/contractAddress/storage/containers/arrayID/0",
			"blcc://eth1.0/contractAddress/storage/containers/arrayID/@",
			"blcc://eth1.0/contractAddress/storage/containers/!/arrayID",
			"blcc://eth1.0/contractAddress/storage/containers/arrayID/#",
			"blcc://eth1.0/contractAddress/storage/containers/arrayID/1",
			"blcc://eth1.0/contractAddress/storage/containers/arrayID/@",
		},
		[]uint32{1, 1, 0, 1, 1, 1, 0, 1},
		[]uint32{0, 0, 1, 0, 0, 0, 1, 0},
		[]bool{false, false, true, false, false, false, true, false},
		[]bool{false, false, false, false, false, false, false, false},
	)
	txs, groups, flags := wrapper.Detect(engine, 8)
	t.Log(txs)
	t.Log(groups)
	t.Log(flags)
}

func TestArbitratorCase1(t *testing.T) {
	engine := wrapper.Start()
	wrapper.Insert(
		engine,
		[]uint32{1, 2, 3, 4},
		[]string{
			"blcc:/eth/address/storage/containers/map/0",
			"blcc:/eth/address/storage/containers/map/0",
			"blcc:/eth/address/storage/containers/map/0",
			"blcc:/eth/address/storage/containers/map/0",
		},
		[]uint32{0, 0, 0, 0},
		[]uint32{1, 1, 1, 1},
		[]bool{false, false, false, false},
		[]bool{true, true, false, false},
	)
	txs, groups, flags := wrapper.Detect(engine, 4)
	t.Log(txs)
	t.Log(groups)
	t.Log(flags)

	left, right := wrapper.ExportTxs(engine)
	t.Log(left)
	t.Log(right)
}

func TestArbitratorCase2(t *testing.T) {
	engine := wrapper.Start()
	wrapper.Insert(
		engine,
		[]uint32{1, 2, 3, 4},
		[]string{
			"blcc:/eth/address/storage/containers/map/0",
			"blcc:/eth/address/storage/containers/map/0",
			"blcc:/eth/address/storage/containers/map/0",
			"blcc:/eth/address/storage/containers/map/0",
		},
		[]uint32{0, 1, 1, 0},
		[]uint32{1, 0, 0, 1},
		[]bool{false, false, false, false},
		[]bool{true, false, false, true},
	)
	txs, groups, flags := wrapper.Detect(engine, 4)
	t.Log(txs)
	t.Log(groups)
	t.Log(flags)

	left, right := wrapper.ExportTxs(engine)
	t.Log(left)
	t.Log(right)
}

func TestArbitratorCase3(t *testing.T) {
	engine := wrapper.Start()
	wrapper.Insert(
		engine,
		[]uint32{1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4},
		[]string{
			"blcc://eth1.0/accounts/Alice/storage/containers/map1/key1",
			"blcc://eth1.0/accounts/Alice/balance",
			"blcc://eth1.0/accounts/Alice/storage/containers/map1/key2",
			"blcc://eth1.0/accounts/Alice/balance",
			"blcc://eth1.0/accounts/Alice/storage/containers/map1/key1",
			"blcc://eth1.0/accounts/Alice/balance",
			"blcc://eth1.0/accounts/Alice/storage/containers/map1/key1",
			"blcc://eth1.0/accounts/Alice/balance",
			"blcc://eth1.0/accounts/Alice/balance",
			"blcc://eth1.0/accounts/Alice/balance",
			"blcc://eth1.0/accounts/Alice/storage/containers/map2/key1",
			"blcc://eth1.0/accounts/Alice/storage/containers/map2/key2",
			"blcc://eth1.0/accounts/Alice/balance",
			"blcc://eth1.0/accounts/Alice/storage/containers/map3/key1",
			"blcc://eth1.0/accounts/Alice/storage/containers/map3/key2",
			"blcc://eth1.0/accounts/Alice/balance",
		},
		[]uint32{0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		[]uint32{1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		[]bool{true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true},
		[]bool{false, true, false, true, false, true, false, true, true, true, false, false, true, false, false, true},
	)
	txs, groups, flags := wrapper.Detect(engine, 16)
	t.Log(txs)
	t.Log(groups)
	t.Log(flags)

	left, right := wrapper.ExportTxs(engine)
	t.Log(left)
	t.Log(right)
}
