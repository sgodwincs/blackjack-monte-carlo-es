constexpr unsigned int aceAltValue{ 11 };
constexpr std::size_t iterationCount{ 1000000000 };
constexpr unsigned int maxValue{ 21 };

std::random_device randomDevice{};
std::mt19937 gen(randomDevice());
std::uniform_int_distribution<unsigned int> playerSumDistribution{ 11, 21 };
std::bernoulli_distribution usableAceDistribution{ 0.5 };
std::bernoulli_distribution initialAction{ 0.5 };
std::uniform_int_distribution<unsigned int> cardDistribution{ 1, 13 };

template <typename T>
void HashCombine(std::size_t& seed, const T& v)
{
	std::hash<T> hasher{};
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

struct PlayerState
{
	unsigned int dealersCard;
	unsigned int sum;
	bool usableAce;

	PlayerState(unsigned int sum, bool usableAce, unsigned int dealersCard) :
		dealersCard{ dealersCard },
		sum{ sum },
		usableAce{ usableAce } { }

	bool operator==(const PlayerState& other) const
	{
		return dealersCard == other.dealersCard &&
			sum == other.sum &&
			usableAce == other.usableAce;
	}
};

using StateActionPair = std::pair<PlayerState, bool>;

class PlayerStateHash
{
public:
	std::size_t operator()(const PlayerState& playerState) const
	{
		std::size_t seed{ 0 };
		HashCombine(seed, playerState.dealersCard);
		HashCombine(seed, playerState.sum);
		HashCombine(seed, playerState.usableAce);
		return seed;
	}
};

using PolicyMap = std::unordered_map<PlayerState, bool, PlayerStateHash>;

class StateActionPairHash
{
public:
	std::size_t operator()(const StateActionPair& stateActionPair) const
	{
		std::size_t seed{ PlayerStateHash{}(stateActionPair.first) };
		HashCombine(seed, stateActionPair.second);
		return seed;
	}
};

using ActionValueMap = std::unordered_map<StateActionPair, double, StateActionPairHash>;
using ReturnsMap = std::unordered_map<StateActionPair, std::size_t, StateActionPairHash>;

struct StateActionInfo
{
	std::vector<StateActionPair> stateActionPairs{};
	std::unordered_set<StateActionPair, StateActionPairHash> stateActionSet{};

	void AddPair(StateActionPair&& stateActionPair)
	{
		if (stateActionSet.find(stateActionPair) == stateActionSet.end())
		{
			stateActionPairs.emplace_back(stateActionPair);
			stateActionSet.insert(stateActionPair);
		}
	}
};

class Player
{
private:
	bool usingAce;
	PlayerState playerState;

public:
	Player(unsigned int sum, bool usableAce, unsigned int dealersCard) :
		usingAce{ usableAce },
		playerState{ sum, usableAce, dealersCard } { }

	void AddCard(unsigned int card)
	{
		if (usingAce && playerState.sum + card > maxValue)
		{
			usingAce = false;
			playerState.sum += 1 - aceAltValue + card;
		}
		else
		{
			playerState.sum += card;
		}
	}

	bool IsBust()
	{
		return GetValue() > maxValue;
	}

	PlayerState GetState()
	{
		return playerState;
	}

	unsigned int GetValue()
	{
		return playerState.sum;
	}

	bool WillHit(PolicyMap& policyMap)
	{
		return policyMap[playerState];
	}
};

class Dealer
{
private:
	std::vector<unsigned int> cards;

public:
	Dealer(std::vector<unsigned int>&& cards) :
		cards{ cards } { }

	void AddCard(unsigned int card)
	{
		cards.emplace_back(card);
	}

	bool IsBust()
	{
		return GetValue() > maxValue;
	}

	unsigned int GetValue()
	{
		unsigned int currentSum{ 0 };
		unsigned int aceCount{ 0 };

		for (auto card : cards)
		{
			if (card == 1)
			{
				++aceCount;
			}
			else
			{
				currentSum += card;
			}
		}

		while (aceCount > 0)
		{
			--aceCount;
			currentSum += aceAltValue;

			if (currentSum > maxValue)
			{
				++aceCount;
				currentSum += aceCount - aceAltValue;
				break;
			}
		}

		return currentSum;
	}

	bool WillHit()
	{
		if (GetValue() >= 17)
		{
			return false;
		}

		return true;
	}
};

int GenerateCard()
{
	auto card{ cardDistribution(gen) };
	return card > 9 ? 10 : card;
}

void EvaluateAndImprovePolicy(
	ActionValueMap& actionValueMap,
	PolicyMap& policyMap,
	ReturnsMap& returnsMap,
	std::vector<StateActionPair>& stateActionPairs,
	int reward)
{
	for (auto& pair : stateActionPairs)
	{
		auto& returnData{ returnsMap[pair] };
		++returnData;
		actionValueMap[pair] = actionValueMap[pair] + ((reward - actionValueMap[pair]) / returnData);

		auto& state{ pair.first };
		bool shouldHit{ false };

		if (actionValueMap[{ state, true }] > actionValueMap[{ state, false }])
		{
			shouldHit = true;
		}

		policyMap[state] = shouldHit;
	}
}

std::vector<ActionValueMap> actionValuesMaps{};
std::vector<ReturnsMap> returnsMaps{};
std::vector<PolicyMap> policyMaps{};

void GenerateStart(
	ActionValueMap& actionValueMap,
	PolicyMap& policyMap,
	ReturnsMap& returnsMap,
	unsigned int lowerBound,
	unsigned int upperBound)
{
	std::uniform_int_distribution<unsigned int> dealersCardDistribution{ lowerBound, upperBound };

	auto playerSum{ playerSumDistribution(gen) };
	auto usableAce{ usableAceDistribution(gen) };
	auto dealersCard1{ dealersCardDistribution(gen) };

	Player player{ playerSum, usableAce, dealersCard1 };
	Dealer dealer{ { dealersCard1 } };

	auto shouldHit{ initialAction(gen) };
	StateActionInfo stateActionInfo{};
	stateActionInfo.AddPair({ player.GetState(), shouldHit });

	if (shouldHit)
	{
		player.AddCard(GenerateCard());

		while (!player.IsBust() && player.WillHit(policyMap))
		{
			stateActionInfo.AddPair({ player.GetState(), true });
			player.AddCard(GenerateCard());
		}
	}

	if (player.IsBust())
	{
		EvaluateAndImprovePolicy(actionValueMap, policyMap, returnsMap, stateActionInfo.stateActionPairs, -1);
		return;
	}
	
	stateActionInfo.AddPair({ player.GetState(), false });
	dealer.AddCard(GenerateCard());

	while (!dealer.IsBust() && dealer.WillHit())
	{
		dealer.AddCard(GenerateCard());
	}

	if (dealer.IsBust() || dealer.GetValue() < player.GetValue())
	{
		EvaluateAndImprovePolicy(actionValueMap, policyMap, returnsMap, stateActionInfo.stateActionPairs, 1);
	}
	else if (dealer.GetValue() > player.GetValue())
	{
		EvaluateAndImprovePolicy(actionValueMap, policyMap, returnsMap, stateActionInfo.stateActionPairs, -1);
	}
	else
	{
		EvaluateAndImprovePolicy(actionValueMap, policyMap, returnsMap, stateActionInfo.stateActionPairs, 0);
	}
}

void PerformMonteCarloES(std::size_t index, unsigned int lowerBound, unsigned int upperBound)
{
	ActionValueMap& actionValueMap{ actionValuesMaps[index] };
	ReturnsMap& returns = { returnsMaps[index] };
	PolicyMap& policyMap = { policyMaps[index] };

	for (std::size_t playerSum{ 11 }; playerSum < 22; ++playerSum)
	{
		for (auto usableAce : { false, true })
		{
			for (std::size_t dealersCard{ lowerBound }; dealersCard < upperBound + 1; ++dealersCard)
			{
				auto playerState{ PlayerState{ static_cast<unsigned int>(playerSum), usableAce, static_cast<unsigned int>(dealersCard) } };
				actionValueMap[std::make_pair(playerState, false)] = 0;
				actionValueMap[std::make_pair(playerState, true)] = 0;
				returns[std::make_pair(playerState, false)] = { 0 };
				returns[std::make_pair(playerState, true)] = { 0 };

				// Default policy.

				if (playerSum > 19)
				{
					policyMap[playerState] = false;
				}
				else
				{
					policyMap[playerState] = true;
				}
			}
		}
	}

	for (std::size_t i{ 0 }; i < iterationCount; ++i)
	{
		GenerateStart(actionValueMap, policyMap, returns, lowerBound, upperBound);
	}
}

int main()
{
	unsigned int coreCount{ std::thread::hardware_concurrency() };

	actionValuesMaps.resize(coreCount);
	returnsMaps.resize(coreCount);
	policyMaps.resize(coreCount);

	std::vector<unsigned int> sections{};
	std::vector<std::thread> threads{};

	unsigned int baseSize{ 10 / coreCount };
	unsigned int remainderNum{ 10 % coreCount };
	unsigned int offset{ 1 };
	std::size_t currentIndex{ 0 };

	for (auto i{ 0U }; i < remainderNum; ++i)
	{
		threads.emplace_back(PerformMonteCarloES, currentIndex, offset, offset + baseSize);
		offset += baseSize + 1;
		++currentIndex;
		sections.emplace_back(offset);
	}

	for (auto i{ 0U }; i < coreCount - remainderNum; ++i)
	{
		threads.emplace_back(PerformMonteCarloES, currentIndex, offset, offset + baseSize - 1);
		offset += baseSize;
		++currentIndex;
		sections.emplace_back(offset);
	}

	for (auto& thread : threads)
	{
		thread.join();
	}

	for (std::size_t playerSum{ 21 }; playerSum > 10; --playerSum)
	{
		std::cout << playerSum << " ";

		for (std::size_t dealersCard{ 1 }; dealersCard < 11; ++dealersCard)
		{
			std::size_t index{};

			for (std::size_t i{ 0 }; i < sections.size(); ++i)
			{
				if (dealersCard < sections[i])
				{
					index = i;
					break;
				}
			}

			std::cout << policyMaps[index][{ static_cast<unsigned int>(playerSum), true, static_cast<unsigned int>(dealersCard) }] << " ";
		}

		std::cout << std::endl;
	}

	std::cout << "   ";

	for (std::size_t dealersCard{ 1 }; dealersCard < 11; ++dealersCard)
	{
		std::cout << dealersCard << " ";
	}

	std::cout << std::endl << std::endl;

	for (std::size_t playerSum{ 21 }; playerSum > 10; --playerSum)
	{
		std::cout << playerSum << " ";

		for (std::size_t dealersCard{ 1 }; dealersCard < 11; ++dealersCard)
		{
			std::size_t index{};

			for (std::size_t i{ 0 }; i < sections.size(); ++i)
			{
				if (dealersCard < sections[i])
				{
					index = i;
					break;
				}
			}

			std::cout << policyMaps[index][{ static_cast<unsigned int>(playerSum), false, static_cast<unsigned int>(dealersCard) }] << " ";
		}

		std::cout << std::endl;
	}

	std::cout << "   ";

	for (std::size_t dealersCard{ 1 }; dealersCard < 11; ++dealersCard)
	{
		std::cout << dealersCard << " ";
	}

	std::cout << std::endl << std::endl;

	char ch;
	std::cin >> ch;
}
