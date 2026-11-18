// Copyright (c) 8888 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "txdb.h"
#include "main.h"
#include "uint256.h"


static const int nCheckpointSpan = 10;

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    //
    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    //
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        (       0, hashGenesisBlock )
        (   25000, uint256("0x44818857f4b0d4a2b87dd67f74f5101992ad2c3ab55fefe97f332025038478a9"))
        (   50000, uint256("0x2c28d0f4b70f40560c94bd157c2fccd82db1406f05e66e4dac814d5945ded03c"))
        (   75000, uint256("0xf6204f0f6fdf6d8bd43b00257ca550a4b6f03db1cb32ff3df4e969c33c298bb8"))
        (  100000, uint256("0x199159719474a1d0a6ffa5c04c693f36e2e54f3d2143f067300b5a327195e1a8"))
        (  125000, uint256("0x5ac7a3b3f7c4e145e9b1c9d0f4afdf04f872d99aa6ec6aae95f1c90356824b87"))
        (  150000, uint256("0x27737709d43d8c9bb25d4eb76b67b72081a86c4392f5004130cfbf25e27cdac1"))
        (  175000, uint256("0x57d63408fd485d0b5e1b3180096cbf599f73a45a0d844c2093e4338fb6d5816d"))
        (  200000, uint256("0xe64361249d060a80056c89ab6ca52f87d322af86e4de63e7edc7a166d2c75883"))
        (  225000, uint256("0xf96f51357327297ed15764c6a91e45c341ecf54bf18197326e955d67d3453403"))
        (  250000, uint256("0x11f911a2acb7c6d615fb8d90d3a722756edf5f93e3bcd71a5ef9e31e7a3e4120"))
        (  275000, uint256("0x7bbc992b1dde489678bf7943d08f4bc88ea43700839e36ce34dc70160593480b"))
        (  300000, uint256("0xa4856b4ae6b0b09554ac5834d8a6fb1b3fb775ffd68c8bdc05da6afaaeea8d4b"))
        (  325000, uint256("0x19e88e360ad9524ba6161f0b9612002559fcc555bf14f06688ce5ec59d5eb9f9"))
        (  350000, uint256("0x0d0412e04cdfb3756258b213024377f8249adef8bb4a2fbd9437dcfdf9da2cd7"))
        (  375000, uint256("0x87386534985bad904b36d7524093d3170aaf8585c2e31e1da8617f8208381a28"))
        (  400000, uint256("0x79483f3cd5d2692c24c88a945b09eea3a54b027cd982fc0e3ccd2341e17345ba"))
        (  425000, uint256("0x80b1f6797d77f607549d83c9d593ea0765da75b74d2c0352fb81fedb83995eb4"))
        (  450000, uint256("0xe73190f4e624ec1a8eb2bf953b0748d875064ce2b81ec5ef718954bad0cd79c9"))
        (  475000, uint256("0xe7b4bc832484a6f290a05e55504beccc88d400e4ddf2ab58c2b1727cba9ad0b2"))
        (  500000, uint256("0x45d3302a377f84c179e6f77ead95821acbd93ce6b7ea5e482edf6ae28808ba3c"))
        (  525000, uint256("0x642e61271a6b462e0b534597b7164ddb850a84529675a3d1a9a188b97392de0a"))
        (  550000, uint256("0xa484c6db670a69980627e09f539b78a8664e14da6b2741671425510ec79c1958"))
        (  575000, uint256("0xa6c6979a7921d69baab6576386b595c262acb938ccdab7708cd211a62c012521"))
        (  600000, uint256("0x9c99cd74521e5187e3f97260701871ab750e97dd108298cb34bffbff2cb6a2c0"))
        (  625000, uint256("0xe1834a9ccd4bc77cc2596a6c3add63d494349e50aabe92644c94cc910f5fec24"))
        (  650000, uint256("0x34c8547c60b3ea5f3a138a2b96667823611889feedff93de9cdde3ecda87051b"))
        (  675000, uint256("0x7aa404069389874a9a7029afc148ce02091cfafdc7881407818430f1e506aee5"))
        (  700000, uint256("0xe0fb3e27af5a39371a7e3d949929f875209db6badc71193c1caa22751f9866db"))
        (  725000, uint256("0xf22a6f280019ef792e316988f3d2a56d7b761922ffe2ec73adaa93b3a03b8269"))
        (  750000, uint256("0x329d41c9a4e8438ef5303667060872fc4a7e232ceeac5fcb53fce85e9a469e7d"))
        (  775000, uint256("0x017d5ffab3377b6451ffb8b708d64584a6da3c498cfe3fa890636de83c35f12a"))
        (  800000, uint256("0x24d9d97c7caaf666ef3fa26cfab2be0a9b5183ebb8377482090ece7c3a49e61f"))
        (  825000, uint256("0x7a364654f01fb37ccf03001cef66303d464ee0cb974f740380477946b80285d8"))
        (  850000, uint256("0xbdd82968ad30705f5fe29f6ad5e0706d5878d765f74418a46fc1e7c200e5dee6"))
        (  875000, uint256("0x6efe23fd551ca680438c778dd88927a4008b583f8bdefa0b5fe30aac472d8f9e"))
        (  900000, uint256("0x93ada7180d3a073d95ff7e421b257935caa2fbdc28bee10654f82a9b26765891"))
        (  925000, uint256("0xa4287412161062d2ce2bf4a698eb0f4497c0f8702ce4ec32fc20d3ca91bbf2c9"))
        (  950000, uint256("0x770db98316a80d64940d241e77214806a08adc1e487a38383e33d8ec36d65cd1"))
        (  975000, uint256("0x502f412f114dca8f2bcc256f99caab043dff7b8132e192b93177bde77e35bc86"))
        ( 1000000, uint256("0xe9044810a25ac48ce0adb3635cf12f7f2bf06f43708f82003975d197943e51de"))
        ( 1025000, uint256("0x4c9a1c708d66c51b76bfcfd1fb865d3f9a349e82f526f1173b66f740a2ef17a6"))
        ( 1050000, uint256("0x8ea66b4032ae8dc2d05e790be911eaf7437cf9cb0e3d7a706ab3d727519fd5f0"))
        ( 1075000, uint256("0xdfa17022ebb6d0f8e1bedd4291172ef2201e38ea5543f6dcbdceedbb0b6f4edd"))
        ( 1100000, uint256("0x8df90ff3c122179b64a8de3fcc1ffa9f7c2150c482b55f7706adac28d23c9d1a"))
        ( 1125000, uint256("0x9a49a805542a1529b61f812af786e43fa9c9a42d3ce8f834ce2db72c4b78a2b8"))
        ( 1150000, uint256("0x9758757abafe1f4720487fdee9b8b67f86d4d7023eee93c34c42f317b616eb18"))
        ( 1175000, uint256("0xc8ccc6cda60ad9f5117779b532257cdb9eb79a02c5f468d113246fab86e27712"))
        ( 1200000, uint256("0x589fc1f4d57c51f3b2a582327e5cd52522034c6d3cd7bb88dbf44ac9849a846e"))
        ( 1225000, uint256("0x9b586476aec76c0203c38c1288d516d6c5c969dc6ebc99d21d15582b155c3c8e"))
        ( 1250000, uint256("0xc9d8b68332ef9b86701e77bb499afbb1163ee1053da6e032aebf6e868a312c40"))
        ( 1275000, uint256("0xdac2baf5528391408cbd42e2873aec00fe6ffcb851b2fc1123795ba06a96f29b"))
        ( 1300000, uint256("0x5312e293da03f7b51f2011e5cd406e77267cdce46ad8da23639c5a18f54b9973"))
        ( 1325000, uint256("0xbd1572b23f4e42ffc3201611cece084ebaa169bfc00e2a58461d49fbc6574a4f"))
        ( 1350000, uint256("0x1da4ecebd8e4ab8a3ab0be2e05c55733cb947c3348e5df45cff0c7ba50b07666"))
        ( 1375000, uint256("0xf460a85e28915d2ded4b9981b05fc6b51568e0301b83d1c1849621adc994460c"))
        ( 1400000, uint256("0xa61533f7c437b78e7f1ddc8b97b514536df997122ed80e65e064bd97c4bf7e05"))
        ( 1425000, uint256("0x86950fedd26b90b8211b317709bf9df15b7835c2f3d8d9a2a99c464793f9dee1"))
        ( 1450000, uint256("0xeeba5ffc15255a5e20e75ebcf45a4e75d41491b88503b561c929c23d2c488da3"))
        ( 1475000, uint256("0x91dec7796c7b547fd55c2bae9b0d963f27fd1222d3b8ed50b48fef2d9296e47f"))
        ( 1500000, uint256("0xce11724551184ab1af9a2a167985dbef761e639b0b6cd27eef66701a7b05e101"))
        ( 1525000, uint256("0xbcd73ccf94c92a6bad751371da26f04c36f9a5de0e06b3d81f072d77cb90a12c"))
        ( 1550000, uint256("0x03734792b7e6b7a81b29c999df38f2ae43a556776c9b1a78c9ce1001446f38a6"))
        ( 1575000, uint256("0xda60731a6e2b17ea43ea54a7ed33478064ede7bba2be18954e3934e5764be080"))
        ( 1600000, uint256("0xd7a4e8551751479aab5d677b1e7f169ca6e20468792775ec8b5f316561448aee"))
        ( 1625000, uint256("0xaf0a4ae86f8347e1feb76b7520e16f9f225e32b042c3a334513d790faa1c9b28"))
        ( 1650000, uint256("0x89b9e1a8c5268463d719382168f3d24f8cf75c216de2c167b9fb47bb56c2bd65"))
        ( 1675000, uint256("0x96b7cc5f6ff0a8a6b45951e352eba98c5b308a8aa621da56107436d1764bf006"))
        ( 1700000, uint256("0x6055fc864e4b5c48b5507d9901ccd4a9df70d98fe34d76aa892507456dd1354f"))
        ( 1725000, uint256("0x10d74c56079bb32318e45aa36271056dd6719764bb55a381d07a5bde47b24fc8"))
        ( 1750000, uint256("0x4c959e1878f51f60c03d9cf584dd6efcc40197fcf38c42a67660d75014ed60a8"))
        ( 1775000, uint256("0x4e81caa273a9348cab92414d4de9a4960066e8396660320733d8a9e500aa8c42"))
        ( 1800000, uint256("0x5d5dc53fe8d4468704c62db25f8932951517bc4af3167f744595f199a8a4e698"))
        ( 1825000, uint256("0x6296d12f1987762494eb2aecbdef3adbaa05b8f9e411e17356f4bc05861cbf9c"))
        ( 1850000, uint256("0x3d2bb6ea8da3356632e12de2973a4ba3da9b549b5be5c55a9f766af587a7b5db"))
        ( 1875000, uint256("0xe1a1faf7cf18d131245de05e370e8e582194ba3005534cda3425f40ed00a792c"))
        ( 1900000, uint256("0x37866b7a21871d0275e1d11d206afe4afe5ad7b2a8ed88083e38b0beaa3c10a5"))
        ( 1925000, uint256("0x2970e38356fb1a3976576382a4f96b045e18d6740eff7586c9b49b6e2a1af7d4"))
        ( 1950000, uint256("0xa4f36993291af7dfc1d1075cd7ee3513dad0740762a6d55ca201bd8dc6590ce5"))
        ( 1975000, uint256("0xfddc416b57702a84388dafc2e119df89b3d4b059b18151554db3dabc4f0e64c0"))
        ( 2000000, uint256("0x9080d2fe37b26ddd4e4192b2c20db757ce81cdc4140222e88e6204855311244a"))
        ( 2025000, uint256("0x21d9b145c35e2f048d79792f9d09b0c94a4b1955fc79db192e8ce85a7d83238e"))
        ( 2050000, uint256("0x9e8515a749db350ee7c10ab693dff9d03b610eec27460bf94769768f1401f8c0"))
        ( 2075000, uint256("0x591c01feb10e385bb86eaedd29c3fdd2009d951f2405d489a3fbdcdbee6b15a8"))
        ( 2100000, uint256("0xd91190f8e26877cdc9c12ee33e76d1f8c2544c7a7fc87600f4aae03fb5fcb41c"))
        ( 2125000, uint256("0x01269be9ecdca9d25e54310a7f03de3034fb889e976a8314e64f5c72acacd447"))
        ( 2150000, uint256("0x0e866e6d091f6ef100edc2e23d10853ec1cdc3195278a44a3f883964f2a7f42b"))
        ( 2175000, uint256("0xa2aaca49a848bb000f3db7204a2cdabee8c4ef14e2833f37b09b979f577d4d3f"))
        ( 2200000, uint256("0x2373d5bc83c42d6a408f7bb42c550963280e4e79470279e97b9987d67e77d003"))
        ( 2225000, uint256("0x94cf24c7069840d4677e0f140a8be30fd77d008cd581b398ffc8abce2cd1f7dd"))
        ( 2250000, uint256("0xaa31f55cc0d21bff6ba8a86c959b6922a32c8cffceffee058b524045712221dc"))
        ( 2275000, uint256("0x9a638776c0305fe501a71eddf48c58a8464dd14b67aa2e27f39229c3a7137718"))
        ( 2300000, uint256("0x8292c711e31ed00b82ce8d9395b3d76787f92d2e5dd8a9425f003b9df3a94f7a"))
        ( 2325000, uint256("0x13dbe3710818d6cf7797b3d1240ac203fd79800c358da58fd5063bad1999f509"))
        ( 2350000, uint256("0x6cc260b595f6e609a0758d595c1528b73864c871f5d9afc3c5928b8618ae467f"))
        ( 2375000, uint256("0x51810b4e3feac6530466f9eb0235ff34b99d77d3ecc52b18dc71ef170b81991d"))
        ( 2400000, uint256("0x98a9e656db642d45e75825ef0d0e5756a85847fa631252f7a9463f2d35107b7d"))
        ( 2425000, uint256("0x0cfc734ad101565377a5ab485d5cc5c74828217761ae21757e4e89201c29b8cc"))
        ( 2450000, uint256("0xed38dff3a2fb04a17b2e4fab352700bf78aef55886e6ed873130fd9dc22ab7dd"))
        ( 2475000, uint256("0x38c82871d7c5b4b39228df8514593336d70b4eaaffaecc3417d6b3be530ecc16"))
        ( 2500000, uint256("0x662055f92c47af2116108b430c504f4459a08043b5cc36af662ea6c7d8331128"))
        ( 2525000, uint256("0x82bf554ed97e5cbf252e191f77e7fb7250dd931961801b21ec4b099024cdc31e"))
        ( 2550000, uint256("0x5e28d93ddb2f0b9ae163851285d4f79086cbbf0b5bca352050f1507860a54fd3"))
        ( 2575000, uint256("0x3348ae104435d44463a5b7f7ae7b610ef3d50ca1db79c2518202e6567b1e5274"))
        ( 2600000, uint256("0x81a0330e07d9a9dc0faf65953e3a5d942b8f53cdc86a23554d40e05f91c1c7a3"))
        ( 2625000, uint256("0xaeff86298f6429b0c84f526865fef0f575a2a0f2293d520dcafcb6357ec0b60e"))
        ( 2650000, uint256("0x8d4a1ef6feff8e88d9ac6ddc896ecc5090e52a049b431ab4ba1f58fbdaefcf00"))
        ( 2675000, uint256("0x04ba09f6467137b2174eea76dbc93e2ff30c7d3fb9dd24459b5b3bb7885b1890"))
        ( 2700000, uint256("0xa2b8bf7879ab7d7133cf5f8d178c138cd4eee4afa94317bf783b488d23a72d45"))
        ( 2725000, uint256("0x0888dfa86a684b3fff715fcdc5238af2d1c72a67b4d186869814a90a4a9b64f9"))
        ( 2750000, uint256("0x0c651319261f87f5e49e3836904c7fb532214c51ef0e83ce8ce88aa4c6a69e56"))
        ( 2775000, uint256("0xd208e3d970e15cb6af2ad9bf110893c0c6287d44ec636fcee204305cca316fa7"))
        ( 2800000, uint256("0x7f221bb194d887db55fa5a0b96d71cca7d7185da1e6f5e1013e4f35af65121da"))
        ( 2825000, uint256("0x705f5eb2da214dbe7aa8937714acbc4e28da492202b5d289b2aa2f5706ae5348"))
        ( 2850000, uint256("0xddcb13c60376575100dcca391f7972de984cce1a16ee201607e7e88870f38a30"))
        ( 2875000, uint256("0x6c8dff630a544cdce5d19565bcc76b3f910cce221023c5c3d8e8356c4f34540e"))
        ( 2900000, uint256("0x617a62cbe2b465cfed92b243ae5967501a2a37122d5bb9a1d91e219f22624be3"))
        ( 2925000, uint256("0x16b891abea914792eb59d030ae6a143fd23d39828fbd439e8576610b267075cd"))
        ( 2950000, uint256("0x8b345fefb1dd3617ea48786a80c147a00afcc796f70938272303442e7a11dc64"))
        ( 2975000, uint256("0xcf8e0e02086d7941a46f4295ba3aded458bee9769b1135b6692885dd4e1c6c69"))
        ( 3000000, uint256("0x4e25e4b1fa83bdde537772c97279a93677ce666748e520c7a80c5b002a9a32d2"))
        ( 3025000, uint256("0xdf910e46100c5f6d0159dfdff8fe12727f6578c17849aaa58f27e3b3fa59e6b1"))
        ( 3050000, uint256("0x0e8122b50f5a502a5509be67a4d93f84a4c32d7c506d125e5c92b13cdebdb5d3"))
        ( 3075000, uint256("0x53f580066cb2a19448f4c8e8e20c5aec864183bc920c4a9fed99cf8a95b9167a"))
        ( 3100000, uint256("0x1f8bddb3e5f72d2df52a99066261a0c3416a8f7da6065492fd542faca761e191"))
        ( 3125000, uint256("0x6bbe866eacade4fe75ece541ee45a9421ca13013e8426040e3fce08149da7585"))
        ( 3150000, uint256("0xaa8a3049d84872a61d8addea1fcf6ee1cbaed79377fca5d44d14fa7b2d44e434"))
        ( 3175000, uint256("0xf6fdd7604b7c2e271253d03b6ed9c017c55c6a70a481ae109de529cfc3f89fc7"))
        ( 3200000, uint256("0x66dfb8e38e4a50f19c0343347f4f2f1b60094315bb8342b7712e9c4119cee0f5"))
        ( 3225000, uint256("0xb3efa58787c06f0904ba34c1fe352d49da6f393d95197f4d42faf5726d6fae27"))
        ( 3250000, uint256("0x0557d09d5c70fd8010552462fd8d25ca82b1dd42d4bed99acfe34f440b009725"))
        ( 3275000, uint256("0xbc6c88a43f74a76b02181e7a281a2c0f135da2b11909ba3f9f8a567e231ebaf4"))
        ( 3300000, uint256("0xd044c79d94d9a81bcb3725cb7323401dd77c23e663284786ffb806cc1beca635"))
        ( 3325000, uint256("0xf0751dd56442c92e08b906da437191dffff0557831d5698a8ad7c1b0d24ed019"))
        ( 3350000, uint256("0xb5beb57e3be0bdbb35133d90082c2fdf009ba44f54e1521141f91d72f08c9e8e"))
        ( 3375000, uint256("0x1988dd6f54085f865ea52ddfe247826259cca1602606414d5b95bce6dabdbe07"))
        ( 3400000, uint256("0xa8de300b1ab9f05790f60b4c96a9cf31f9b3505e26b9513d532b7f7bb8b7cad9"))
        ( 3425000, uint256("0x75f90bc14381bbb6638a8faad0e6acb14e2e10d0a4d45dc750fd0aee605c7ed0"))
        ( 3450000, uint256("0x9142c3e2224e05756a7af7fa99f42eadfe60f176027472750d7aea93b8617d0a"))
        ( 3475000, uint256("0x2c539e399e6f56cf387d32adbee121f1a681d027854afd064136df0b9f11e542"))
        ( 3500000, uint256("0xbe353c26f96e28ea96b8e806426ce70dfe69ff243c836799f4ed6055c005815a"))
        ( 3525000, uint256("0x528e5e32a2f3d3d0439b45f9411f5692062da00fc55852586bbc9512fbcb8101"))
        ( 3550000, uint256("0xbd4cef47cf9e3aef10c2c10a8731f8c793c1a23a8c8bb1824045f2ce1ff66123"))
        ( 3575000, uint256("0x9f07efba787f5d3c49c0dd6f920afaf158d0facc9cc2561dabad148b504d6294"))
        ( 3600000, uint256("0xcc1b80bc8a9058c0d364dd6a064f68ae8914a6516546cb674678d8cd12329ea7"))
        ( 3625000, uint256("0x150c1c27adbad3ba6d517d9360bc7c3642fc91cf28f63921e11a63dc44aee6e3"))
        ( 3650000, uint256("0xe0ffa5622f3f47e81dbf0a2202808cb215d33f0db2dd8f43783747836957c0fe"))
        ( 3675000, uint256("0x80c5ee56552d38d3b0f6095119db42ba98e0782dc3d1605096cf7a5b079f82c2"))
        ( 3700000, uint256("0xd5ad0eda6b9bbea86d4fb3a8f323bc1fad940a54f9961ff758109e5cbd361d00"))
        ( 3725000, uint256("0xd1d7f76a4a84b727d1cf13bf4ee03a9a396b90144b5d4d748f3dc451bbe68bd8"))
        ( 3750000, uint256("0xf7f91ca85c669dd74e3f3369679ac0352420fa5faf3e7c24f8d37abd5cd174b4"))
        ( 3775000, uint256("0x92454b837749502488e42f463ac1dfacc8cb40fb55b852a354f4a50a18ee8da0"))
        ( 3800000, uint256("0xf2763aac1b64d57ceabd086195d11ba7d44fb5edd8a706188c3e0777283cbd75"))
        ( 3825000, uint256("0xa6755178601cc0568de1c1fc644bc63e7bb4414700a67ec6c5c566c7112abae7"))
        ( 3850000, uint256("0xae63d6d4b9f00ebb6460418484cfd2f12acb0b5b4fb1cbd23e8072a10578484f"))
        ( 3875000, uint256("0xf33dcc97a8bccf6628b73f2c6e0fe2cfd9919d8867e118e06b5790f8c7826fc7"))
        ( 3900000, uint256("0xf1820373d9faaf936fdc0e4271a3d88b72ce130d67efd99c3ff962189a44cc8d"))
        ( 3925000, uint256("0x10da1b075262286862f6a35aafac898ea6aad12275e623e25db8a015ee480810"))
        ( 3950000, uint256("0xac0358c83225c5b35a134425284670ecd7a914d9bf666a159bc9af921b8ae37f"))
        ( 3975000, uint256("0xece50544666fac068680de466418b51958cf01d890f66362f078fd852cdea673"))
        ( 4000000, uint256("0x2f5e764500d6ac092f1d802530e302abf5fa66fe334374049092dc005ceec04c"))
        ( 4025000, uint256("0x7acad42cee120ff29e9225844165c532d582b71b6358924a35e59d1bd2719349"))
        ( 4050000, uint256("0x5ff3b95b7c6c0828881188506e9d9245cfc14b4b938b545d79ceb180737a8d3f"))
        ( 4075000, uint256("0x9d1a248a0fb3b5e04f16fdfa0f37a5b9f72cc2972539e02108d827b6d1295a21"))
        ( 4100000, uint256("0x440cb17b68003a5bd125f2019fb32c5017eea625d339531828c53a91744c0396"))
        ( 4125000, uint256("0xc7727bc26df9f965ec3e7ee4ca581db972d76497c94f6a34fa69e8dd91446c8a"))
        ( 4150000, uint256("0xbf22328b5ddb3878a564b389c59aec586bf2d8cae41f295d52862e68fa073bdf"))
        ( 4175000, uint256("0x4d14b26adc32a6d2dcd2be5844f1a71137ad016b6b8e14245e764775b04a5dca"))
        ( 4200000, uint256("0x1cbe8f3fb6c4ae0a361806158cbc165ddf0076db8f91991d2bc3db14c45b2b0c"))
        ( 4225000, uint256("0xd85b3b25de186aff7a665cd686d5d0570bbd7b45acb1cb507daf41f005c148eb"))
        ( 4250000, uint256("0xf7731478face39d46ae7d95f2a05a8d77170fb972480ec30817cf11f49b10808"))
        ( 4275000, uint256("0xf219b1e34d17c068ef5982eaab24ee1056fc7a69cc1d187beefe601ba2f0dc9b"))
        ( 4300000, uint256("0x95e706db76e36b331845f829c0cc0c117beb94f76ac4450fac72f84d0e1e3997"))
        ( 4325000, uint256("0x0118f492b57615c2608b24b035ea7583ce95ccdc691dfe658172520e586c63ab"))
        ( 4350000, uint256("0xd9c0032f30fe3b56931ae37cfd99ac4dac681be25981c3aa592338b649608024"))
        ( 4375000, uint256("0xb7701e461d72eaf573ae4ead74f2d42a81d09107d83344a7837bab64d9448b79"))
        ( 4400000, uint256("0x8eaeac8bda7dbaef13423293d9ae2889fb4a76d6d46311586415ffb48d84704e"))
        ( 4425000, uint256("0x996006de863fd63a9440dc2a5fc9321db3a649f9c7c5794fae67c8e5d85dfeb9"))
        ( 4450000, uint256("0xe8590ef08fd335a92d4906574008218a9f58554082930cd028f4fcd72187bba3"))
        ( 4475000, uint256("0x7172d140bc0b246d2e6a904d8bfbb1859ffd3f7637296f449ecd04a40ffbb681"))
        ( 4500000, uint256("0x0cb2de2dad09e40e17c6e2a927dfca5fd2c53a116180e8fcda892b449db0357b"))
        ( 4525000, uint256("0xb4ebdb35b44f7c0412d60e62f45530a008859d551bb951b293f7b9fc6981daaa"))
        ( 4550000, uint256("0x0f5e807a06395ade8a9dbb57e14f48df615fa7c757a65c9753b9f54b911903b8"))
        ( 4575000, uint256("0xfed3c175f8d9686503d90c52a551a335f368c5102105a1ed93d45c25f2c65da7"))
        ( 4600000, uint256("0x8026f02fbdc57cfb02c8efeea72a6a725eaaed7da979ed387efdc1673a022220"))
        ( 4625000, uint256("0x1fa93c5172f70442da15016e4182664df50d4879c00f8e29e948a6f54c28c596"))
        ( 4650000, uint256("0x8ea0a2366c83083d0412de80855949a956baa5b618076c86fd0473eae2eca089"))
        ( 4675000, uint256("0xd0c47c713c747ed67724094b832d86c0ab80d3243bb8c17c17dee8ad4e95e953"))
        ( 4700000, uint256("0xf7379937cb47a42748b833c67854e3fc069f2ca68f4fe6a92ec6f4eb883787e9"))
        ( 4725000, uint256("0x629adf7c63de8e29358a62b81872e5ea91ec7d9e71aff5b3fa4f67e66c10fa5c"))
        ( 4750000, uint256("0x1c05ee74179b2d42d413a32c96c93d332d76d1bed62962f215f3df8f8d552b3b"))
        ( 4775000, uint256("0xb65d55b74869679041febcbff03ca5d3fc63f84f6ee2e87c7753eb169f9b340b"))
        ( 4800000, uint256("0x9d45e42ec43ce68a21fbdf65708cc5a10cf666b2a4e5e88d0561b600b84da10c"))
        ( 4825000, uint256("0x7891436982900631e7e0890587cbeded96c52a8730de4f23318df266ce95f64b"))
        ( 4850000, uint256("0x99427c4b87b344104fbe21c292ebeb79883b72eeea391b0a13f3b35b0b6812b1"))
        ( 4875000, uint256("0x9b580ecbd7a1a2a85e158b8f24abae22015a130e621b9030ab0e66bfde40d852"))
        ( 4900000, uint256("0x994f945669bbbf40301e348e49b5b1a0bc2579065ef364eb5ed7912c08cec8b8"))
        ( 4925000, uint256("0x59e121b4a6223a439f8a18f78d79168f5ca29bff7cdb64994cbe769b101ae65f"))
        ( 4950000, uint256("0x9f728fef526911d6173f0fe9bab9f0108cf757e0ed8711600ddd44c0394d1f5e"))
        ( 4975000, uint256("0x7f0e41b2e6b128110777e404bf834591735ae0a8daa0b464c5da192ff14e1e11"))
        ( 5000000, uint256("0x954a145ca20e5e4a5858a1615abfe92c09cabcca0f9544dd7d8297010fe138cc"))
        ( 5025000, uint256("0x0d2d50d485364bfe1b752a7334070d3e660673a61499be7129261d479dd4e50f"))
        ( 5050000, uint256("0xb8dcfe78dbdf64c8259f3694fb3ccf67edefa89c2cfef292cd1c82bdb5fe288e"))
        ( 5075000, uint256("0x92c97f000e2d12e10e4a6656b502b401bc7275d2af1d15c211ef4d76d2153f56"))
        ( 5100000, uint256("0xc882cbd3fe635dc28656c0b6531bcca777e7522da1df41de1d62a913450d6bde"))
        ( 5125000, uint256("0x98383fa9ff78a88e2179b115268000631b98afd90ee13368bf9824781e381150"))
        ( 5150000, uint256("0x6d379be1224f8401afa911fdf0e367d6eaa355d8a1bb3472999cd41f90b1e1e9"))
        ( 5175000, uint256("0xfc045333451530ce2d4ccee591e89ca59648f8429aeb399d48bffe947edc5f4c"))
        ( 5200000, uint256("0xb07efaa5d38fff7571a483d557e4ce5d4127c83ce31a3dca8ae8f5b210bbd37f"))
        ( 5225000, uint256("0x286669a219f7a54c5add7fcca674311b3b1a73c43463db5ed91c1f2908a04872"))
        ( 5250000, uint256("0x3b8cd06a680a2c097070e6014e4f0685728310614b693e57b5e18131554b2949"))
        ( 5275000, uint256("0x49947329441800274070c6e4a9983ed8f3a008100f55ab6a50244c61992ca65b"))
        ( 5300000, uint256("0x14330c806ebe0c6da65f26abd278e4c4098da3d02f1cc77267cf79d93c4483f9"))
        ( 5325000, uint256("0xdf8334c38cddc091cd3aebab56e79a312fddc29fe136b68ffbc94a7d0d93a744"))
        ( 5350000, uint256("0x1f586452cf56ab3654e938dd50fca21292dafcdc6295349e408070770329765c"))
        ( 5375000, uint256("0xa2e3d05ae0c1da0eb049e8d71b85c07385aeeadfd276e9229d9a2db5156591ee"))
        ( 5400000, uint256("0x692a969e27d15bf2340b5b2f8466d2399842e5450f00dd08c8d280f456f1f2c7"))
        ( 5425000, uint256("0x85080e03dd1da10fd3f5585691be95313a767ea39660b470d4b1f82e159be438"))
        ( 5450000, uint256("0xe476cda3e933f08a032ecf9aba03160f101ae39a2e45fb6e18e005e1ba7caa31"))
        ( 5475000, uint256("0x5d3af2a84f9a61a77b0dc50d0b43f41b8e53381f6dba0f458da77b69cad77bfa"))
        ( 5500000, uint256("0x704a5e723298373cbf7240b1ffd94473495adbd27a9c37d4ae7dce44088db26d"))
        ( 5525000, uint256("0x04249800659360a5b09e92f6dbea6b6e94ef2bd1bd9bfe53f3e7dfe18682079f"))
        ( 5550000, uint256("0x60fac44e9291b4053b0cb5e423621527a2ce8a28ce376aba6986ba7984646195"))
        ( 5575000, uint256("0x3145826f803ed6677a1cdfdd9fcbc49a50f11d7b268553ec8acf4225ef98fbf5"))
        ( 5600000, uint256("0xc03b516ab76d6d9b31cf8cb70f2f46cba8c9f9e975737c2a1456a675fc55a4f2"))
        ( 5625000, uint256("0xa915218e482333c3198c7cfe648a31d0e8261a04a5176a68a2ae2ef45966ba5f"))
        ( 5650000, uint256("0x754d9e0a460af5eee2ac65be4d5498f3d2825eec14d47cc699de61bf160fd44a"))
        ( 5675000, uint256("0x8cac3b6980d6d5895bc51cc58847f7ef1567f2ee5099917ebfc84621e2eede8e"))
        ( 5700000, uint256("0x5175212cac73ebd04b66d5ddf18e791a068194801bea554cb36e88288dc77e75"))
        ( 5725000, uint256("0x790a4b506a2475de28ef7de6e9bc7f68885c2d189e78cebe6315e54f185a0b6a"))
    ;

    // TestNet has no checkpoints
    static MapCheckpoints mapCheckpointsTestnet =
        boost::assign::map_list_of
        ( 0, hashGenesisBlockTestNet )
    ;

    bool CheckHardened(int nHeight, const uint256& hash)
    {
        MapCheckpoints& checkpoints = (fTestNet ? mapCheckpointsTestnet : mapCheckpoints);

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    int GetTotalBlocksEstimate()
    {
        MapCheckpoints& checkpoints = (fTestNet ? mapCheckpointsTestnet : mapCheckpoints);

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const BlockMap& mapBlockIndex)
    {
        MapCheckpoints& checkpoints = (fTestNet ? mapCheckpointsTestnet : mapCheckpoints);

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            BlockMap::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }

    // ppcoin: synchronized checkpoint (centrally broadcasted)
    uint256 hashSyncCheckpoint = 0;
    uint256 hashPendingCheckpoint = 0;
    CSyncCheckpoint checkpointMessage;
    CSyncCheckpoint checkpointMessagePending;
    uint256 hashInvalidCheckpoint = 0;
    CCriticalSection cs_hashSyncCheckpoint;

    // ppcoin: get last synchronized checkpoint
    CBlockIndex* GetLastSyncCheckpoint()
    {
        LOCK(cs_hashSyncCheckpoint);
        if (!mapBlockIndex.count(hashSyncCheckpoint))
            error("GetSyncCheckpoint: block index missing for current sync-checkpoint %s", hashSyncCheckpoint.ToString().c_str());
        else
            return mapBlockIndex[hashSyncCheckpoint];
        return NULL;
    }

    // ppcoin: only descendant of current sync-checkpoint is allowed
    bool ValidateSyncCheckpoint(uint256 hashCheckpoint)
    {
        if (!mapBlockIndex.count(hashSyncCheckpoint))
            return error("ValidateSyncCheckpoint: block index missing for current sync-checkpoint %s", hashSyncCheckpoint.ToString().c_str());
        if (!mapBlockIndex.count(hashCheckpoint))
            return error("ValidateSyncCheckpoint: block index missing for received sync-checkpoint %s", hashCheckpoint.ToString().c_str());

        CBlockIndex* pindexSyncCheckpoint = mapBlockIndex[hashSyncCheckpoint];
        CBlockIndex* pindexCheckpointRecv = mapBlockIndex[hashCheckpoint];

        if (pindexCheckpointRecv->nHeight <= pindexSyncCheckpoint->nHeight)
        {
            // Received an older checkpoint, trace back from current checkpoint
            // to the same height of the received checkpoint to verify
            // that current checkpoint should be a descendant block
            CBlockIndex* pindex = pindexSyncCheckpoint;
            while (pindex->nHeight > pindexCheckpointRecv->nHeight)
                if (!(pindex = pindex->pprev))
                    return error("ValidateSyncCheckpoint: pprev null - block index structure failure");
            if (pindex->GetBlockHash() != hashCheckpoint)
            {
                hashInvalidCheckpoint = hashCheckpoint;
                return error("ValidateSyncCheckpoint: new sync-checkpoint %s is conflicting with current sync-checkpoint %s", hashCheckpoint.ToString().c_str(), hashSyncCheckpoint.ToString().c_str());
            }
            return false; // ignore older checkpoint
        }

        // Received checkpoint should be a descendant block of the current
        // checkpoint. Trace back to the same height of current checkpoint
        // to verify.
        CBlockIndex* pindex = pindexCheckpointRecv;
        while (pindex->nHeight > pindexSyncCheckpoint->nHeight)
            if (!(pindex = pindex->pprev))
                return error("ValidateSyncCheckpoint: pprev2 null - block index structure failure");
        if (pindex->GetBlockHash() != hashSyncCheckpoint)
        {
            hashInvalidCheckpoint = hashCheckpoint;
            return error("ValidateSyncCheckpoint: new sync-checkpoint %s is not a descendant of current sync-checkpoint %s", hashCheckpoint.ToString().c_str(), hashSyncCheckpoint.ToString().c_str());
        }
        return true;
    }

    bool WriteSyncCheckpoint(const uint256& hashCheckpoint)
    {
        CTxDB txdb;
        txdb.TxnBegin();
        if (!txdb.WriteSyncCheckpoint(hashCheckpoint))
        {
            txdb.TxnAbort();
            return error("WriteSyncCheckpoint(): failed to write to db sync checkpoint %s", hashCheckpoint.ToString().c_str());
        }
        if (!txdb.TxnCommit())
            return error("WriteSyncCheckpoint(): failed to commit to db sync checkpoint %s", hashCheckpoint.ToString().c_str());

        Checkpoints::hashSyncCheckpoint = hashCheckpoint;
        return true;
    }

    bool AcceptPendingSyncCheckpoint()
    {
        LOCK(cs_hashSyncCheckpoint);
        if (hashPendingCheckpoint != 0 && mapBlockIndex.count(hashPendingCheckpoint))
        {
            if (!ValidateSyncCheckpoint(hashPendingCheckpoint))
            {
                hashPendingCheckpoint = 0;
                checkpointMessagePending.SetNull();
                return false;
            }

            CTxDB txdb;
            CBlockIndex* pindexCheckpoint = mapBlockIndex[hashPendingCheckpoint];
            if (!pindexCheckpoint->IsInMainChain())
            {
                CBlock block;
                if (!block.ReadFromDisk(pindexCheckpoint))
                    return error("AcceptPendingSyncCheckpoint: ReadFromDisk failed for sync checkpoint %s", hashPendingCheckpoint.ToString().c_str());
                if (!block.SetBestChain(txdb, pindexCheckpoint))
                {
                    hashInvalidCheckpoint = hashPendingCheckpoint;
                    return error("AcceptPendingSyncCheckpoint: SetBestChain failed for sync checkpoint %s", hashPendingCheckpoint.ToString().c_str());
                }
            }

            if (!WriteSyncCheckpoint(hashPendingCheckpoint))
                return error("AcceptPendingSyncCheckpoint(): failed to write sync checkpoint %s", hashPendingCheckpoint.ToString().c_str());
            hashPendingCheckpoint = 0;
            checkpointMessage = checkpointMessagePending;
            checkpointMessagePending.SetNull();
            printf("AcceptPendingSyncCheckpoint : sync-checkpoint at %s\n", hashSyncCheckpoint.ToString().c_str());
            // relay the checkpoint
            if (!checkpointMessage.IsNull())
            {
                BOOST_FOREACH(CNode* pnode, vNodes)
                    checkpointMessage.RelayTo(pnode);
            }
            return true;
        }
        return false;
    }

    // Automatically select a suitable sync-checkpoint
    uint256 AutoSelectSyncCheckpoint()
    {
        const CBlockIndex *pindex = pindexBest;
        // Search backward for a block within max span and maturity window
        while (pindex->pprev && (pindex->GetBlockTime() + nCheckpointSpan * nTargetSpacing > pindexBest->GetBlockTime() || pindex->nHeight + nCheckpointSpan > pindexBest->nHeight))
            pindex = pindex->pprev;
        return pindex->GetBlockHash();
    }

    // Check against synchronized checkpoint
    bool CheckSync(const uint256& hashBlock, const CBlockIndex* pindexPrev)
    {
        if (fTestNet) return true; // Testnet has no checkpoints
        int nHeight = pindexPrev->nHeight + 1;

        LOCK(cs_hashSyncCheckpoint);
        // sync-checkpoint should always be accepted block
        assert(mapBlockIndex.count(hashSyncCheckpoint));
        const CBlockIndex* pindexSync = mapBlockIndex[hashSyncCheckpoint];

        if (nHeight > pindexSync->nHeight)
        {
            // Fast path: when both the sync-checkpoint and the new block's parent
            // are on our main chain, the parent descends from the checkpoint (the
            // main chain is a single line). Without this, every new block walked
            // back to the checkpoint one block at a time, which gets slower the
            // further the chain grows past the checkpoint.
            if (pindexSync->IsInMainChain() && pindexPrev->IsInMainChain())
                return true;

            // trace back to same height as sync-checkpoint
            const CBlockIndex* pindex = pindexPrev;
            while (pindex->nHeight > pindexSync->nHeight)
                if (!(pindex = pindex->pprev))
                    return error("CheckSync: pprev null - block index structure failure");
            if (pindex->nHeight < pindexSync->nHeight || pindex->GetBlockHash() != hashSyncCheckpoint)
                return false; // only descendant of sync-checkpoint can pass check
        }
        if (nHeight == pindexSync->nHeight && hashBlock != hashSyncCheckpoint)
            return false; // same height with sync-checkpoint
        if (nHeight < pindexSync->nHeight && !mapBlockIndex.count(hashBlock))
            return false; // lower height than sync-checkpoint
        return true;
    }

    bool WantedByPendingSyncCheckpoint(uint256 hashBlock)
    {
        LOCK(cs_hashSyncCheckpoint);
        if (hashPendingCheckpoint == 0)
            return false;
        if (hashBlock == hashPendingCheckpoint)
            return true;
        if (mapOrphanBlocks.count(hashPendingCheckpoint)
            && hashBlock == WantedByOrphan(mapOrphanBlocks[hashPendingCheckpoint]))
            return true;
        return false;
    }

    // ppcoin: reset synchronized checkpoint to last hardened checkpoint
    bool ResetSyncCheckpoint()
    {
        LOCK(cs_hashSyncCheckpoint);
        const uint256& hash = mapCheckpoints.rbegin()->second;
        if (mapBlockIndex.count(hash) && !mapBlockIndex[hash]->IsInMainChain())
        {
            // checkpoint block accepted but not yet in main chain
            printf("ResetSyncCheckpoint: SetBestChain to hardened checkpoint %s\n", hash.ToString().c_str());
            CTxDB txdb;
            CBlock block;
            if (!block.ReadFromDisk(mapBlockIndex[hash]))
                return error("ResetSyncCheckpoint: ReadFromDisk failed for hardened checkpoint %s", hash.ToString().c_str());
            if (!block.SetBestChain(txdb, mapBlockIndex[hash]))
            {
                return error("ResetSyncCheckpoint: SetBestChain failed for hardened checkpoint %s", hash.ToString().c_str());
            }
        }
        else if(!mapBlockIndex.count(hash))
        {
            // checkpoint block not yet accepted
            hashPendingCheckpoint = hash;
            checkpointMessagePending.SetNull();
            printf("ResetSyncCheckpoint: pending for sync-checkpoint %s\n", hashPendingCheckpoint.ToString().c_str());
        }

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, mapCheckpoints)
        {
            const uint256& hash = i.second;
            if (mapBlockIndex.count(hash) && mapBlockIndex[hash]->IsInMainChain())
            {
                if (!WriteSyncCheckpoint(hash))
                    return error("ResetSyncCheckpoint: failed to write sync checkpoint %s", hash.ToString().c_str());
                printf("ResetSyncCheckpoint: sync-checkpoint reset to %s\n", hashSyncCheckpoint.ToString().c_str());
                return true;
            }
        }

        return false;
    }

    void AskForPendingSyncCheckpoint(CNode* pfrom)
    {
        LOCK(cs_hashSyncCheckpoint);
        if (pfrom && hashPendingCheckpoint != 0 && (!mapBlockIndex.count(hashPendingCheckpoint)) && (!mapOrphanBlocks.count(hashPendingCheckpoint)))
            pfrom->AskFor(CInv(MSG_BLOCK, hashPendingCheckpoint));
    }

    bool SetCheckpointPrivKey(std::string strPrivKey)
    {
        // Test signing a sync-checkpoint with genesis block
        CSyncCheckpoint checkpoint;
        checkpoint.hashCheckpoint = !fTestNet ? hashGenesisBlock : hashGenesisBlockTestNet;
        CDataStream sMsg(SER_NETWORK, PROTOCOL_VERSION);
        sMsg << (CUnsignedSyncCheckpoint)checkpoint;
        checkpoint.vchMsg = std::vector<unsigned char>(sMsg.begin(), sMsg.end());

        std::vector<unsigned char> vchPrivKey = ParseHex(strPrivKey);
        CKey key;
        key.SetPrivKey(CPrivKey(vchPrivKey.begin(), vchPrivKey.end())); // if key is not correct openssl may crash
        if (!key.Sign(Hash(checkpoint.vchMsg.begin(), checkpoint.vchMsg.end()), checkpoint.vchSig))
            return false;

        // Test signing successful, proceed
        CSyncCheckpoint::strMasterPrivKey = strPrivKey;
        return true;
    }

    bool SendSyncCheckpoint(uint256 hashCheckpoint)
    {
        CSyncCheckpoint checkpoint;
        checkpoint.hashCheckpoint = hashCheckpoint;
        CDataStream sMsg(SER_NETWORK, PROTOCOL_VERSION);
        sMsg << (CUnsignedSyncCheckpoint)checkpoint;
        checkpoint.vchMsg = std::vector<unsigned char>(sMsg.begin(), sMsg.end());

        if (CSyncCheckpoint::strMasterPrivKey.empty())
            return error("SendSyncCheckpoint: Checkpoint master key unavailable.");
        std::vector<unsigned char> vchPrivKey = ParseHex(CSyncCheckpoint::strMasterPrivKey);
        CKey key;
        key.SetPrivKey(CPrivKey(vchPrivKey.begin(), vchPrivKey.end())); // if key is not correct openssl may crash
        if (!key.Sign(Hash(checkpoint.vchMsg.begin(), checkpoint.vchMsg.end()), checkpoint.vchSig))
            return error("SendSyncCheckpoint: Unable to sign checkpoint, check private key?");

        if(!checkpoint.ProcessSyncCheckpoint(NULL))
        {
            printf("WARNING: SendSyncCheckpoint: Failed to process checkpoint.\n");
            return false;
        }

        // Relay checkpoint
        {
            LOCK(cs_vNodes);
            BOOST_FOREACH(CNode* pnode, vNodes)
                checkpoint.RelayTo(pnode);
        }
        return true;
    }

    // Is the sync-checkpoint outside maturity window?
    bool IsMatureSyncCheckpoint()
    {
        LOCK(cs_hashSyncCheckpoint);
        // sync-checkpoint should always be accepted block
        assert(mapBlockIndex.count(hashSyncCheckpoint));
        const CBlockIndex* pindexSync = mapBlockIndex[hashSyncCheckpoint];
        return (nBestHeight >= pindexSync->nHeight + nCoinbaseMaturity ||
                pindexSync->GetBlockTime() + nStakeMinAge < GetAdjustedTime());
    }
}

// ppcoin: sync-checkpoint master key
const std::string CSyncCheckpoint::strMasterPubKey = "0446d5e358df4936adfedbbe4f01e624770d16c79cc96d0b068bba677a2217ba55fb24a637d0d04adcafce8e9b2c53db65d684e308ff402fe2be5dbfe569de11f2";

std::string CSyncCheckpoint::strMasterPrivKey = "";

// ppcoin: verify signature of sync-checkpoint message
bool CSyncCheckpoint::CheckSignature()
{
    CKey key;
    if (!key.SetPubKey(ParseHex(CSyncCheckpoint::strMasterPubKey)))
        return error("CSyncCheckpoint::CheckSignature() : SetPubKey failed");
    if (!key.Verify(Hash(vchMsg.begin(), vchMsg.end()), vchSig))
        return error("CSyncCheckpoint::CheckSignature() : verify signature failed");

    // Now unserialize the data
    CDataStream sMsg(vchMsg, SER_NETWORK, PROTOCOL_VERSION);
    sMsg >> *(CUnsignedSyncCheckpoint*)this;
    return true;
}

// ppcoin: process synchronized checkpoint
bool CSyncCheckpoint::ProcessSyncCheckpoint(CNode* pfrom)
{
    if (!CheckSignature())
        return false;

    LOCK(Checkpoints::cs_hashSyncCheckpoint);
    if (!mapBlockIndex.count(hashCheckpoint))
    {
        // We haven't received the checkpoint chain, keep the checkpoint as pending
        Checkpoints::hashPendingCheckpoint = hashCheckpoint;
        Checkpoints::checkpointMessagePending = *this;
        printf("ProcessSyncCheckpoint: pending for sync-checkpoint %s\n", hashCheckpoint.ToString().c_str());
        // Ask this guy to fill in what we're missing
        if (pfrom)
        {
            pfrom->PushGetBlocks(pindexBest, hashCheckpoint);
            // ask directly as well in case rejected earlier by duplicate
            // proof-of-stake because getblocks may not get it this time
            pfrom->AskFor(CInv(MSG_BLOCK, mapOrphanBlocks.count(hashCheckpoint)? WantedByOrphan(mapOrphanBlocks[hashCheckpoint]) : hashCheckpoint));
        }
        return false;
    }

    if (!Checkpoints::ValidateSyncCheckpoint(hashCheckpoint))
        return false;

    CTxDB txdb;
    CBlockIndex* pindexCheckpoint = mapBlockIndex[hashCheckpoint];
    if (!pindexCheckpoint->IsInMainChain())
    {
        // checkpoint chain received but not yet main chain
        CBlock block;
        if (!block.ReadFromDisk(pindexCheckpoint))
            return error("ProcessSyncCheckpoint: ReadFromDisk failed for sync checkpoint %s", hashCheckpoint.ToString().c_str());
        if (!block.SetBestChain(txdb, pindexCheckpoint))
        {
            Checkpoints::hashInvalidCheckpoint = hashCheckpoint;
            return error("ProcessSyncCheckpoint: SetBestChain failed for sync checkpoint %s", hashCheckpoint.ToString().c_str());
        }
    }

    if (!Checkpoints::WriteSyncCheckpoint(hashCheckpoint))
        return error("ProcessSyncCheckpoint(): failed to write sync checkpoint %s", hashCheckpoint.ToString().c_str());
    Checkpoints::checkpointMessage = *this;
    Checkpoints::hashPendingCheckpoint = 0;
    Checkpoints::checkpointMessagePending.SetNull();
    printf("ProcessSyncCheckpoint: sync-checkpoint at %s\n", hashCheckpoint.ToString().c_str());
    return true;
}
