package wrapper_test

import (
	"fmt"
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

	//t.Log(insertions, times)
	txs, groups, flags := wrapper.DetectLegacy(engine, []uint32{2, 3})
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
	txs, groups, flags := wrapper.DetectLegacy(engine, []uint32{1, 2, 3, 4})
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
	txs, groups, flags := wrapper.DetectLegacy(engine, []uint32{1, 2, 3, 4})
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
	txs, groups, flags := wrapper.DetectLegacy(engine, []uint32{1, 2, 3, 4})
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
	txs, groups, flags := wrapper.DetectLegacy(engine, []uint32{0, 1, 2, 3})
	t.Log(txs)
	t.Log(groups)
	t.Log(flags)

	left, right := wrapper.ExportTxs(engine)
	t.Log(left)
	t.Log(right)
}

func TestArbitratorCase5(t *testing.T) {
	//capacity := 5000000
	txs := []uint32{256, 256, 256, 256, 256, 256, 256, 256, 256, 512, 512, 512, 512, 512, 512, 512, 512, 512, 768, 768, 768, 768, 768, 768, 768, 768, 768, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1280, 1280, 1280, 1280, 1280, 1280, 1280, 1280, 1280, 1536, 1536, 1536, 1536, 1536, 1536, 1536, 1536, 1536, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2304, 2304, 2304, 2304, 2304, 2304, 2304, 2304, 2304, 2560, 2560, 2560, 2560, 2560, 2560, 2560, 2560, 2560}
	paths := []string{
		"blcc://eth1.0/account/c8bc50ca2443f4ce0ebf1bc9396b7f53f62e9c13/balance", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/code", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/", "blcc://eth1.0/account/c8bc50ca2443f4ce0ebf1bc9396b7f53f62e9c13/nonce", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/balance", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x0000000000000000000000000000000000000000000000000000000000000001", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x88340702fdbd7738965b9bd706de3eff9e797234878784c7318fac1b79b76b1e", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x0000000000000000000000000000000000000000000000000000000000000002", "blcc://eth1.0/account/bdc90f6cf7e6d159b75e07738a15228bd7c15b90/balance", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/balance", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/code", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x0000000000000000000000000000000000000000000000000000000000000001", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0xe2d6c937648d64e70fdfb83e4e14afcd07724be179a5313b7fb3c184ac77707a", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x0000000000000000000000000000000000000000000000000000000000000002", "blcc://eth1.0/account/c8bc50ca2443f4ce0ebf1bc9396b7f53f62e9c13/nonce", "blcc://eth1.0/account/bdc90f6cf7e6d159b75e07738a15228bd7c15b90/balance", "blcc://eth1.0/account/c8bc50ca2443f4ce0ebf1bc9396b7f53f62e9c13/balance", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x0000000000000000000000000000000000000000000000000000000000000001", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x0000000000000000000000000000000000000000000000000000000000000002", "blcc://eth1.0/account/bdc90f6cf7e6d159b75e07738a15228bd7c15b90/balance", "blcc://eth1.0/account/c8bc50ca2443f4ce0ebf1bc9396b7f53f62e9c13/nonce", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/balance", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/code", "blcc://eth1.0/account/c8bc50ca2443f4ce0ebf1bc9396b7f53f62e9c13/balance", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0xbb0a2c83b60e6f1409e494efe2d15316b0389f341e64208cecba87c2da3f2242", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/balance", "blcc://eth1.0/account/bdc90f6cf7e6d159b75e07738a15228bd7c15b90/balance", "blcc://eth1.0/account/c8bc50ca2443f4ce0ebf1bc9396b7f53f62e9c13/balance", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/code", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x0000000000000000000000000000000000000000000000000000000000000001", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0xa87d3c8ca0e70c22ffa9f5314cb3148569da501271d90a59c89f9909ac85b6e3", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x0000000000000000000000000000000000000000000000000000000000000002", "blcc://eth1.0/account/c8bc50ca2443f4ce0ebf1bc9396b7f53f62e9c13/nonce", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/balance", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/code", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x0000000000000000000000000000000000000000000000000000000000000001", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x0000000000000000000000000000000000000000000000000000000000000002", "blcc://eth1.0/account/bdc90f6cf7e6d159b75e07738a15228bd7c15b90/balance", "blcc://eth1.0/account/c8bc50ca2443f4ce0ebf1bc9396b7f53f62e9c13/balance", "blcc://eth1.0/account/c8bc50ca2443f4ce0ebf1bc9396b7f53f62e9c13/nonce", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x192c86887aedac2bf64f797044d294c54919d357d966ac6516438a8e4a0c44f0", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/code", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x0000000000000000000000000000000000000000000000000000000000000001", "blcc://eth1.0/account/bdc90f6cf7e6d159b75e07738a15228bd7c15b90/balance", "blcc://eth1.0/account/c8bc50ca2443f4ce0ebf1bc9396b7f53f62e9c13/nonce", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/balance", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x0000000000000000000000000000000000000000000000000000000000000002", "blcc://eth1.0/account/c8bc50ca2443f4ce0ebf1bc9396b7f53f62e9c13/balance", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x34ecdb6e06521145112bdbce068e51528a56e81b401826114c4638b2e8e47aaa", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x390b2bb6c406667f97c20a12c299d8dc9ab1d5637899b928c99c3a375a070464", "blcc://eth1.0/account/c8bc50ca2443f4ce0ebf1bc9396b7f53f62e9c13/nonce", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/balance", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x0000000000000000000000000000000000000000000000000000000000000001", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x0000000000000000000000000000000000000000000000000000000000000002", "blcc://eth1.0/account/bdc90f6cf7e6d159b75e07738a15228bd7c15b90/balance", "blcc://eth1.0/account/c8bc50ca2443f4ce0ebf1bc9396b7f53f62e9c13/balance", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/code", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x0000000000000000000000000000000000000000000000000000000000000002", "blcc://eth1.0/account/c8bc50ca2443f4ce0ebf1bc9396b7f53f62e9c13/balance", "blcc://eth1.0/account/c8bc50ca2443f4ce0ebf1bc9396b7f53f62e9c13/nonce", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/balance", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x0000000000000000000000000000000000000000000000000000000000000001", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0xb6e835ab136780ee02f9bc13147ed8f3d4d18b3a13f9202be7839a4ad5c97fe3", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/code", "blcc://eth1.0/account/bdc90f6cf7e6d159b75e07738a15228bd7c15b90/balance", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/balance", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x0000000000000000000000000000000000000000000000000000000000000001", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x544d522fdc16da1a2dff4cb361dd66c239af99b58792fc57d15ec4132516c2df", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x0000000000000000000000000000000000000000000000000000000000000002", "blcc://eth1.0/account/bdc90f6cf7e6d159b75e07738a15228bd7c15b90/balance", "blcc://eth1.0/account/c8bc50ca2443f4ce0ebf1bc9396b7f53f62e9c13/balance", "blcc://eth1.0/account/c8bc50ca2443f4ce0ebf1bc9396b7f53f62e9c13/nonce", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/code", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/", "blcc://eth1.0/account/c8bc50ca2443f4ce0ebf1bc9396b7f53f62e9c13/nonce", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/balance", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x0000000000000000000000000000000000000000000000000000000000000002", "blcc://eth1.0/account/bdc90f6cf7e6d159b75e07738a15228bd7c15b90/balance", "blcc://eth1.0/account/c8bc50ca2443f4ce0ebf1bc9396b7f53f62e9c13/balance", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/code", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x0000000000000000000000000000000000000000000000000000000000000001", "blcc://eth1.0/account/309eced01e331f3128f34932f5971e4127d2c6e6/storage/native/0x6c94ebc1b4d36522fbf4edcd76eb7efd585e0f676ec02651839c995303866f72"}
	reads := []uint32{0, 2, 0, 0, 0, 2, 2, 2, 0, 0, 2, 2, 2, 0, 2, 0, 0, 0, 2, 0, 2, 0, 0, 0, 2, 0, 2, 0, 0, 0, 2, 2, 2, 0, 2, 0, 0, 2, 2, 2, 0, 0, 0, 2, 0, 2, 2, 0, 0, 0, 0, 2, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 0, 0, 2, 0, 0, 0, 2, 2, 2, 0, 0, 2, 2, 2, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0, 0, 2, 2, 2}
	writes := []uint32{3, 0, 2, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 2, 1, 1, 1, 3, 0, 2, 1, 1, 1, 1, 0, 3, 1, 1, 1, 3, 0, 0, 1, 2, 1, 1, 1, 0, 0, 1, 1, 3, 1, 1, 2, 0, 0, 1, 1, 1, 2, 1, 3, 1, 1, 1, 1, 0, 1, 1, 3, 0, 2, 2, 1, 3, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 3, 1, 0, 2, 1, 1, 2, 1, 1, 3, 0, 0, 1}
	composites := []bool{true, false, true, true, true, false, false, false, true, true, false, false, false, true, false, true, true, true, false, true, false, true, true, true, false, true, false, true, true, true, false, false, false, true, false, true, true, false, false, false, true, true, true, false, true, false, false, true, true, true, true, false, true, false, false, true, true, false, false, true, true, false, true, true, false, true, true, true, false, false, false, true, true, false, false, false, true, true, true, false, true, true, true, true, false, true, true, false, false, false}

	engine := wrapper.Start()
	wrapper.Insert(
		engine,
		txs,
		paths,
		reads,
		writes,
		composites,
	)
	group := [][]uint32{{768}, {1792}, {2304}, {2560}, {256}, {512}, {1024}, {1280}, {1536}, {2048}}
	txs, g, flags, _, _ := wrapper.Detect(engine, group)
	fmt.Printf("txs=%v\n", txs)
	fmt.Printf("g=%v\n", g)
	fmt.Printf("flags=%v\n", flags)
	t.Log(txs)
	t.Log(g)
	t.Log(flags)

	left, right := wrapper.ExportTxs(engine)
	t.Log(left)
	t.Log(right)
}
