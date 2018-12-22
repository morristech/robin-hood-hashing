#include "test_base.h"

#include <array>
#include <fstream>
#include <iomanip>

template <typename T>
T rotl(T const x, int k) {
	return (x << k) | (x >> (8 * sizeof(T) - k));
}

class Avalanche {
public:
	template <typename Op>
	void eval_input(uint64_t input, Op const& op) {
		uint64_t const output = op(input);

		for (int bit = 0; bit < 64; ++bit) {
			auto const input_flipped = input ^ (UINT64_C(1) << (63 - bit));

			// generate output with flipped bit
			auto has_flipped = op(input_flipped) ^ output;
			for (int i = 63; i >= 0; --i) {
				if (has_flipped & 1) {
					++m_flip_count[bit * 64 + i];
				}
				has_flipped >>= 1;
			}
		}
	}

	template <typename Op>
	void eval(uint64_t num_iters, Op const& op) {
		for (uint64_t i = 0; i < num_iters; ++i) {
			eval_input(m_rng(), op);
			eval_input(m_count, op);
			eval_input(m_count << 32, op);
			m_count += 3;
		}
	}

	// calculate current root mean squared error
	double rms() const {
		int const expected_flips = m_count / 2;
		double sumtotal2 = 0;
		for (size_t y = 0; y < 64 * 64; ++y) {
			double e = m_flip_count[y] - expected_flips;
			sumtotal2 += e * e;
		}
		return std::sqrt(sumtotal2 / (64 * 64));
	}

	void save(std::string filename_ppm, size_t scaling = 8) {
		// source: https://www.kennethmoreland.com/color-advice/
		static const uint8_t bent_cool_warm_rgb[] = {
			85,  72,  193, 86,  74,  194, 87,  75,  194, 88,  77,  195, 88,  78,  196, 89,  79,  196, 90,  81,  197, 90,  82,  197, 91,  84,  198,
			92,  85,  199, 93,  87,  199, 94,  88,  200, 94,  90,  200, 95,  91,  201, 96,  92,  202, 97,  94,  202, 98,  95,  203, 98,  97,  203,
			99,  98,  204, 100, 99,  204, 101, 101, 205, 102, 102, 206, 103, 104, 206, 104, 105, 207, 104, 106, 207, 105, 108, 208, 106, 109, 208,
			107, 110, 209, 108, 112, 209, 109, 113, 210, 110, 115, 210, 111, 116, 211, 112, 117, 211, 113, 119, 212, 114, 120, 212, 115, 121, 213,
			116, 123, 213, 117, 124, 214, 118, 125, 214, 119, 127, 215, 120, 128, 215, 121, 129, 215, 122, 131, 216, 123, 132, 216, 124, 134, 217,
			125, 135, 217, 126, 136, 218, 127, 138, 218, 128, 139, 218, 129, 140, 219, 130, 142, 219, 132, 143, 220, 133, 144, 220, 134, 146, 221,
			135, 147, 221, 136, 148, 221, 137, 150, 222, 138, 151, 222, 140, 152, 222, 141, 154, 223, 142, 155, 223, 143, 156, 224, 144, 158, 224,
			146, 159, 224, 147, 160, 225, 148, 162, 225, 149, 163, 225, 151, 164, 226, 152, 166, 226, 153, 167, 226, 154, 168, 227, 156, 170, 227,
			157, 171, 227, 158, 172, 228, 160, 174, 228, 161, 175, 228, 162, 176, 229, 164, 177, 229, 165, 179, 229, 166, 180, 230, 168, 181, 230,
			169, 183, 230, 170, 184, 230, 172, 185, 231, 173, 187, 231, 175, 188, 231, 176, 189, 232, 177, 191, 232, 179, 192, 232, 180, 193, 232,
			182, 194, 233, 183, 196, 233, 185, 197, 233, 186, 198, 234, 188, 200, 234, 189, 201, 234, 191, 202, 234, 192, 204, 235, 194, 205, 235,
			195, 206, 235, 197, 207, 235, 198, 209, 236, 200, 210, 236, 201, 211, 236, 203, 213, 236, 205, 214, 237, 206, 215, 237, 208, 216, 237,
			209, 218, 237, 211, 219, 238, 213, 220, 238, 214, 221, 238, 216, 223, 238, 217, 224, 239, 219, 225, 239, 221, 227, 239, 222, 228, 239,
			224, 229, 240, 226, 230, 240, 227, 232, 240, 229, 233, 240, 231, 234, 241, 233, 235, 241, 234, 237, 241, 236, 238, 241, 238, 239, 242,
			240, 240, 242, 241, 242, 242, 242, 242, 241, 242, 240, 239, 241, 239, 237, 241, 237, 235, 240, 236, 232, 240, 234, 230, 239, 233, 228,
			239, 231, 226, 239, 230, 224, 238, 228, 221, 238, 226, 219, 237, 225, 217, 237, 223, 215, 237, 222, 213, 236, 220, 211, 236, 219, 209,
			235, 217, 207, 235, 216, 205, 234, 214, 203, 234, 213, 200, 234, 211, 198, 233, 210, 196, 233, 208, 194, 232, 207, 192, 232, 205, 190,
			231, 204, 188, 231, 202, 186, 231, 201, 184, 230, 199, 182, 230, 198, 180, 229, 196, 178, 229, 195, 176, 228, 193, 175, 228, 192, 173,
			228, 190, 171, 227, 188, 169, 227, 187, 167, 226, 185, 165, 226, 184, 163, 225, 182, 161, 225, 181, 159, 224, 179, 158, 224, 178, 156,
			224, 176, 154, 223, 175, 152, 223, 173, 150, 222, 171, 148, 222, 170, 147, 221, 168, 145, 221, 167, 143, 220, 165, 141, 220, 164, 140,
			219, 162, 138, 219, 161, 136, 219, 159, 134, 218, 157, 133, 218, 156, 131, 217, 154, 129, 217, 153, 128, 216, 151, 126, 216, 149, 124,
			215, 148, 123, 215, 146, 121, 214, 145, 119, 214, 143, 118, 213, 141, 116, 213, 140, 115, 212, 138, 113, 212, 137, 111, 211, 135, 110,
			211, 133, 108, 210, 132, 107, 210, 130, 105, 209, 129, 104, 209, 127, 102, 208, 125, 101, 208, 124, 99,  207, 122, 98,  207, 120, 96,
			206, 119, 95,  206, 117, 93,  205, 115, 92,  205, 114, 91,  204, 112, 89,  203, 110, 88,  203, 109, 86,  202, 107, 85,  202, 105, 84,
			201, 103, 82,  201, 102, 81,  200, 100, 80,  200, 98,  78,  199, 96,  77,  198, 95,  76,  198, 93,  74,  197, 91,  73,  197, 89,  72,
			196, 87,  71,  196, 86,  69,  195, 84,  68,  194, 82,  67,  194, 80,  66,  193, 78,  65,  193, 76,  63,  192, 74,  62,  191, 72,  61,
			191, 70,  60,  190, 68,  59,  189, 66,  58,  189, 64,  57,  188, 62,  56,  188, 59,  55,  187, 57,  54,  186, 55,  53,  186, 52,  52,
			185, 50,  50,  184, 47,  49,  184, 45,  49,  183, 42,  48,  182, 39,  47,  182, 36,  46,  181, 33,  45,  181, 29,  44,  180, 25,  43,
			179, 21,  42,  178, 15,  41,  178, 8,   40,  177, 1,   39};
		static_assert(sizeof(bent_cool_warm_rgb) == 3 * 256, "colormap size incorrect");

		std::ofstream fout(filename_ppm, std::ios::binary);
		fout << "P6\n" << (64 * scaling) << " " << (64 * scaling) << "\n255\n";
		for (size_t y = 0; y < 64; ++y) {
			std::vector<uint8_t> rgb_row;
			for (size_t x = 0; x < 64; ++x) {
				// add 1 so max is actually below 256
				size_t color_idx = (m_flip_count[y * 64 + x] * 256) / (m_count + 1);
				for (size_t s = 0; s < scaling; ++s) {
					rgb_row.push_back(bent_cool_warm_rgb[color_idx * 3]);
					rgb_row.push_back(bent_cool_warm_rgb[color_idx * 3 + 1]);
					rgb_row.push_back(bent_cool_warm_rgb[color_idx * 3 + 2]);
				}
			}
			for (size_t s = 0; s < scaling; ++s) {
				fout.write(reinterpret_cast<const char*>(rgb_row.data()), rgb_row.size());
			}
		}
	}

private:
	std::array<int, 64 * 64> m_flip_count{};
	Rng m_rng{};
	uint64_t m_count{};
};

#if 0
template <typename Op>
void avalanche(std::string img_filename, size_t num_flips, Op const& op) {
	for (size_t y = 0; y < 64; ++y) {
		for (size_t x = 0; x < 64; ++x) {
			std::cout << std::dec << std::setfill(' ') << std::setw(3) << (data[y * 64 + x] * 100 / num_flips);
		}
		std::cout << std::endl;
	}
}
#endif

#define UNLIKELY(x) __builtin_expect((x), 0)

template <typename U = uint64_t>
class RandomBool {
public:
	template <typename Rng>
	bool operator()(Rng& rng) {
		if (UNLIKELY(1 == m_rand)) {
			m_rand = std::uniform_int_distribution<U>{}(rng) | s_mask_left1;
		}
		bool const ret = m_rand & 1;
		m_rand >>= 1;
		return ret;
	}

private:
	static constexpr const U s_mask_left1 = U(1) << (sizeof(U) * 8 - 1);
	U m_rand = 1;
};

// 0xd555555555bdef77 111.167
// 0x55555555576f776f 111.14
// 0x5555555557777777 110.803
// 0xdadf257d2af3ebb3 0xd18773185b2b6a69 64.8124
// 0x7d00e6b53b6b5cdb 0x46f40f64faad2cb5 64.6763
TEST_CASE("avalanching", "[!hide]") {
	Rng rng(std::random_device{}());
	RandomBool<> rbool;
	double best_rms = (std::numeric_limits<double>::max)();
	std::array<uint64_t, 3> best_factors = {0xf32caab6cb5656ab, 0x9554aaa9aab55aab, 59};
	auto current_factors = best_factors;
	while (true) {

		Avalanche avalanche;
		avalanche.eval(100, [&current_factors](uint64_t h) {
			h *= current_factors[0];
			h ^= h >> current_factors[2];
			h *= current_factors[1];
			// h ^= h >> 33;
			return h;
		});

		double current_rms = avalanche.rms();
		if (current_rms < best_rms) {
			avalanche.save("test.ppm", 8);
			std::cout << "0x" << std::hex << std::setfill('0') << std::setw(16) << current_factors[0] << " 0x" << std::hex << std::setfill('0')
					  << std::setw(16) << current_factors[1] << " " << current_rms << std::dec << " " << current_factors[2] << std::endl;
			best_rms = current_rms;
			best_factors = current_factors;
		}

		current_factors = best_factors;

		while (rbool(rng)) {
			current_factors[0] ^= (UINT64_C(1) << rng(64));
		}
		while (rbool(rng)) {
			current_factors[1] ^= (UINT64_C(1) << rng(64));
		}

		if (rng(16)) {
			current_factors[2] = rng(63);
		}
	}
}