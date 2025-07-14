#include <random>

class RandomEngine {
public:
    template <std::integral T>
    static T GetRandomRange(T min, T max) {
        std::uniform_int_distribution<T> dist(min, max);
        return dist(MersenneTwister);
    }

    template <std::floating_point T>
    static T GetRandomRange(T min, T max) {
        std::uniform_real_distribution<T> dist(min, max);
        return dist(MersenneTwister);
    }

	static std::mt19937& GetEngine() {
		return MersenneTwister;
	}

private:
	inline static std::random_device rd{};
	inline static std::mt19937 MersenneTwister{ rd() };
};