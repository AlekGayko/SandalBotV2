#ifndef PRECOMPUTEDMAGICS_H
#define PRECOMPUTEDMAGICS_H

#include <cstdint>
#include <vector>

namespace SandalBot {

	class PrecomputedMagics {
	private:
		// Magic Numbers
		uint64_t orthogonalMagics[64] { 9222238992490216896ULL, 14555654121090864632ULL, 13544132741376902154ULL, 7025624378269270116ULL, 16381843578018484987ULL, 18059393113545408125ULL, 14552969406677812391ULL, 13120574539353453305ULL, 10024652856517698217ULL, 2718927102778132419ULL, 5149416267332971932ULL, 14892133925010504892ULL, 6864309449176154492ULL, 16350068759259571012ULL, 2706902229429942429ULL, 18026831610546011761ULL, 5214640611558625309ULL, 8068912718637888210ULL, 6078150226365970059ULL, 8481489593965881063ULL, 10806754542236793564ULL, 2708339630728156018ULL, 12389217390138704731ULL, 9973852494190343735ULL, 17449592415955330508ULL, 9860417695546147670ULL, 17807237003954522973ULL, 6013354839270866609ULL, 3190154166972551181ULL, 4282798829008141793ULL, 200546004457162994ULL, 12801580392322315230ULL, 11919838892714361152ULL, 9266249823710981234ULL, 6112860337687516945ULL, 2604139749388352318ULL, 15159884555973480780ULL, 13032606230223260423ULL, 4811764303914850901ULL, 7595736094930903883ULL, 2934949575098192596ULL, 14141486660886073344ULL, 3476484300324293761ULL, 7004691459998608878ULL, 16082943213102022608ULL, 8661058006428169317ULL, 3458834056355856799ULL, 10604587542355509244ULL, 5511099370773106520ULL, 6448012022138372352ULL, 16128392037010901504ULL, 18366243025382788608ULL, 10442611154696876033ULL, 1526219133187542819ULL, 7247585580073207228ULL, 6095993393002365882ULL, 8829365849805027062ULL, 2606708742896799962ULL, 10020411209037163674ULL, 9031189455634213118ULL, 7257211265169427654ULL, 16859788191231848242ULL, 12941444287740115780ULL, 4438557872706330590ULL };
		uint64_t diagonalMagics[64] { 2502349624283097235ULL, 11043467321474647217ULL, 15682091370055775344ULL, 3984646600960553589ULL, 10377741116788028858ULL, 7075841757963391300ULL, 1062258574023332480ULL, 9212481048514735676ULL, 7297966747818850985ULL, 11347935643973309809ULL, 3312125413124460164ULL, 3994567664716406276ULL, 10034307195039759434ULL, 9423363399531150024ULL, 7990037761501575198ULL, 7021897648429007762ULL, 14076388294507702420ULL, 12094943125597808513ULL, 9204778503947632448ULL, 334658158656480149ULL, 16272361794963013204ULL, 1374024160897133955ULL, 18139798987682615612ULL, 3240410188264423738ULL, 4595519674042187267ULL, 9805894176247507001ULL, 12082020717491938045ULL, 17994389357673944065ULL, 16146754444649588332ULL, 3424690912617738155ULL, 10967750135165054835ULL, 15781752833788770817ULL, 17986298381405755026ULL, 18334670688246082457ULL, 3747604200739723940ULL, 5582835472970136134ULL, 11715737124972044498ULL, 9336241046287547404ULL, 17385864194688148674ULL, 6079637550790973304ULL, 17477818636279582130ULL, 3555010550336301987ULL, 1113730446681776397ULL, 4601683408429400201ULL, 4026349199224500213ULL, 18387326259163786827ULL, 13471033986868310649ULL, 11247350495703753562ULL, 4656286822743886099ULL, 18013209080356836177ULL, 14986183212871114575ULL, 14532844199656797956ULL, 6341845687492049285ULL, 3580665891282403731ULL, 4446941560364347557ULL, 17709857600175900721ULL, 8720635721062059597ULL, 12519972861825906729ULL, 6493387679277327416ULL, 16388411090020491765ULL, 7903856953147483560ULL, 11069457155342024249ULL, 10225600112766264093ULL, 11557011938342712892ULL };
		// Right Shifts
		int orthogonalShifts[64] { 51, 52, 52, 52, 52, 52, 52, 51, 52, 53, 53, 53, 53, 53, 53, 52, 52, 53, 53, 53, 53, 53, 53, 52, 52, 53, 53, 53, 53, 53, 53, 52, 52, 53, 53, 53, 53, 53, 53, 52, 52, 53, 53, 53, 53, 53, 53, 52, 52, 53, 53, 53, 53, 53, 53, 52, 51, 52, 52, 52, 52, 52, 52, 51 };
		int diagonalShifts[64] { 57, 58, 58, 58, 58, 58, 58, 57, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 56, 56, 56, 56, 58, 58, 58, 58, 56, 54, 54, 56, 58, 58, 58, 58, 56, 54, 54, 56, 58, 58, 58, 58, 56, 56, 56, 56, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 57, 58, 58, 58, 58, 58, 58, 57 };
		// Maximum Indexes
		int maxOrthogonalIndexes[64] { 8190, 4091, 4093, 4094, 4094, 4094, 4094, 8188, 4095, 2045, 2046, 2046, 2046, 2045, 2046, 4094, 4024, 2046, 2046, 2046, 2046, 2046, 2046, 4091, 4094, 2043, 2046, 2045, 2046, 2046, 2046, 4094, 4094, 2046, 2046, 2046, 2045, 2044, 2045, 4094, 4092, 2046, 2045, 2044, 2046, 2046, 2046, 4094, 4091, 2047, 2046, 2045, 2044, 2046, 2046, 4094, 8190, 4094, 4094, 4095, 4095, 4091, 4094, 8190 };
		int maxDiagonalIndexes[64] { 125, 59, 62, 62, 63, 61, 62, 126, 60, 62, 60, 63, 61, 62, 62, 61, 61, 61, 254, 254, 253, 254, 61, 63, 60, 61, 255, 1016, 1022, 253, 62, 61, 62, 57, 254, 1020, 1019, 255, 61, 62, 62, 62, 255, 247, 254, 253, 63, 61, 62, 62, 62, 58, 60, 61, 62, 59, 127, 62, 63, 61, 63, 62, 60, 117 };

		// Move Maps
		uint64_t* orthogonalMoves[64];
		uint64_t* diagonalMoves[64];
	public:
		PrecomputedMagics();
		~PrecomputedMagics();
		void addOrthogonalMoves(int square, std::vector<uint64_t>& blockers, std::vector<uint64_t>& movementBoards);
		void addDiagonalMoves(int square, std::vector<uint64_t>& blockers, std::vector<uint64_t>& movementBoards);
		uint64_t getOrthogonalMovement(const int square, const uint64_t blockers) const {
			return orthogonalMoves[square][(blockers * orthogonalMagics[square]) >> orthogonalShifts[square]];
		}
		uint64_t getDiagonalMovement(const int square, const uint64_t blockers) const {
			return diagonalMoves[square][(blockers * diagonalMagics[square]) >> diagonalShifts[square]];
		}
	};

}

#endif // !PRECOMPUTEDMAGICS_H
