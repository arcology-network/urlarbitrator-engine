package wrapper_test

import (
	"testing"

	"github.com/arcology-network/urlarbitrator-engine/go-wrapper"
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
		[]bool{false, false, false, false, false, false, false, false},
	)
	txs, groups, flags := wrapper.Detect(engine, []uint32{2, 3})
	t.Log(txs)
	t.Log(groups)
	t.Log(flags)
	if len(txs) != 0 || len(groups) != 0 || len(flags) != 0 {
		t.Fail()
	}
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
		[]bool{true, true, false, false},
	)
	txs, groups, flags := wrapper.Detect(engine, []uint32{1, 2, 3, 4})
	t.Log(txs)
	t.Log(groups)
	t.Log(flags)

	left, right := wrapper.ExportTxs(engine)
	t.Log(left)
	t.Log(right)
	if len(txs) != 2 || len(groups) != 2 || len(flags) != 2 || len(left) != 5 || len(right) != 5 {
		t.Fail()
	}
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
		[]bool{true, false, false, true},
	)
	txs, groups, flags := wrapper.Detect(engine, []uint32{1, 2, 3, 4})
	t.Log(txs)
	t.Log(groups)
	t.Log(flags)

	left, right := wrapper.ExportTxs(engine)
	t.Log(left)
	t.Log(right)
	if len(txs) != 2 || len(groups) != 2 || len(flags) != 2 || len(left) != 4 || len(right) != 4 {
		t.Fail()
	}
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
		[]bool{false, true, false, true, false, true, false, true, true, true, false, false, true, false, false, true},
	)
	txs, groups, flags := wrapper.Detect(engine, []uint32{1, 2, 3, 4})
	t.Log(txs)
	t.Log(groups)
	t.Log(flags)

	left, right := wrapper.ExportTxs(engine)
	t.Log(left)
	t.Log(right)
	if len(left) != 1 || len(right) != 1 {
		t.Fail()
	}
}

func TestArbitratorCase4(t *testing.T) {
	capacity := 5000000
	txs := make([]uint32, 6, capacity)
	txs[0], txs[1], txs[2], txs[3], txs[4], txs[5] = 0, 0, 1, 1, 2, 3
	paths := make([]string, 6, capacity)
	paths[0], paths[1], paths[2], paths[3], paths[4], paths[5] =
		"blcc://eth1.0/accounts/Alice/storage/containers/map1/key1",
		"blcc://eth1.0/accounts/Alice/balance",
		"blcc://eth1.0/accounts/Alice/storage/containers/map1/key1",
		"blcc://eth1.0/accounts/Alice/balance",
		"blcc://eth1.0/accounts/Alice/balance",
		"blcc://eth1.0/accounts/Alice/balance"
	reads := make([]uint32, 6, capacity)
	reads[0], reads[1], reads[2], reads[3], reads[4], reads[5] = 0, 0, 0, 0, 0, 0
	writes := make([]uint32, 6, capacity)
	writes[0], writes[1], writes[2], writes[3], writes[4], writes[5] = 1, 1, 1, 1, 1, 1
	composites := make([]bool, 6, capacity)
	composites[0], composites[1], composites[2], composites[3], composites[4], composites[5] = false, true, false, true, true, true

	engine := wrapper.Start()
	wrapper.Insert(
		engine,
		// []uint32{0, 0, 1, 1, 2, 3},
		txs,
		// []string{
		// 	"blcc://eth1.0/accounts/Alice/storage/containers/map1/key1",
		// 	"blcc://eth1.0/accounts/Alice/balance",
		// 	"blcc://eth1.0/accounts/Alice/storage/containers/map1/key1",
		// 	"blcc://eth1.0/accounts/Alice/balance",
		// 	"blcc://eth1.0/accounts/Alice/balance",
		// 	"blcc://eth1.0/accounts/Alice/balance",
		// },
		paths,
		// []uint32{0, 0, 0, 0, 0, 0},
		reads,
		// []uint32{1, 1, 1, 1, 1, 1},
		writes,
		// []bool{false, true, false, true, true, true},
		composites,
	)
	txs, groups, flags := wrapper.Detect(engine, []uint32{0, 1, 2, 3})
	t.Log(txs)
	t.Log(groups)
	t.Log(flags)

	left, right := wrapper.ExportTxs(engine)
	t.Log(left)
	t.Log(right)
}
