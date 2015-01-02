#include <iostream>
#include <sstream>
#include <random>
#include <chrono>
#include <algorithm>

namespace std {
    template <typename T>
    constexpr typename std::underlying_type<T>::type enum_value(T val) {
        return static_cast<typename std::underlying_type<T>::type>(val);
    }
}

enum class Suit : uint8_t {
    SPADES = 0,
    HEARTS = 1,
    DIAMONDS = 2,
    CLUBS = 3
};

enum class CardValue : uint8_t {
    UNKNOWN = 0,
    ACE = 1,
    TWO = 2,
    THREE = 3,
    FOUR = 4,
    FIVE = 5,
    SIX = 6,
    SEVEN = 7,
    EIGHT = 8,
    NINE = 9,
    TEN = 10,
    JACK = 11,
    QUEEN = 12,
    KING = 13,
    EMPTY = 14    
};

class Card {
private:
    uint8_t rawCard;
public:
    Card() : rawCard(0) {}
    Card(CardValue value, Suit suit) : rawCard((std::enum_value(value) << 2) | std::enum_value(suit)) {}
    Card(const Card& copy) : rawCard(copy.rawCard) {}
    inline Suit getSuit() const {
        return static_cast<Suit>(rawCard & 0b00000011);
    }
    inline CardValue getValue() const {
        return static_cast<CardValue>(rawCard >> 2);
    }
    Card& operator=(const Card& copy) {
        rawCard = copy.rawCard;
        return *this;
    }
    inline bool isKnown() const { return getValue() != CardValue::UNKNOWN && getValue() != CardValue::EMPTY; }
    static Card UNKNOWN;
    static Card EMPTY;
    inline bool operator==(const Card& other) const { 
        return getValue() == other.getValue() && (!isKnown() || getSuit() == other.getSuit());
    }
    inline bool operator!=(const Card& other) const { return !(*this == other); }
};

Card Card::UNKNOWN;
Card Card::EMPTY(CardValue::EMPTY, Suit::SPADES);

std::ostream& operator<<(std::ostream& stream, const Card& card) {
    switch(card.getValue()) {
    case CardValue::ACE:
        stream << "A";
        break;
    case CardValue::KING:
        stream << "K";
        break;
    case CardValue::QUEEN:
        stream << "Q";
        break;
    case CardValue::JACK:
        stream << "J";
        break;
    case CardValue::TEN:
        stream << "T";
        break;
    case CardValue::UNKNOWN:
        stream << "[]";
        break;
    case CardValue::EMPTY:
        stream << "--";
        break;
    default:
        stream << static_cast<unsigned>(std::enum_value(card.getValue()));
    }
    if(card.isKnown()) {
        switch(card.getSuit()) {
        case Suit::SPADES:
            stream << "S";//"\u2664";
            break;
        case Suit::HEARTS:
            stream << "H";//"\u2661";
            break;
        case Suit::DIAMONDS:
            stream << "D";//"\u2662";
            break;
        case Suit::CLUBS:
            stream << "C";//"\u2667";
            break;
        }
    }
    return stream;
}

class Deck {
private:
    std::vector<Card> cards;
    unsigned seed;
public:
    Deck(unsigned seed) : seed(seed) {
        auto rand = std::default_random_engine(seed);
        for(Suit suit : { Suit::SPADES, Suit::HEARTS, Suit::DIAMONDS, Suit::CLUBS }) {
            for(CardValue value : { CardValue::ACE, CardValue::TWO, CardValue::THREE, CardValue::FOUR, CardValue::FIVE, CardValue::SIX, CardValue::SEVEN, CardValue::EIGHT, CardValue::NINE, CardValue::TEN, CardValue::JACK, CardValue::QUEEN, CardValue::KING }) {
                cards.emplace_back(value, suit);
            }
        }
        std::shuffle(cards.begin(), cards.end(), rand);
    }
    Deck() : Deck(std::chrono::system_clock::now().time_since_epoch().count()) {}
    inline const Card& operator[](size_t index) const {
        return cards[index];
    }
    typedef std::vector<Card>::const_iterator const_iterator;
    inline const_iterator begin() const {
        return cards.begin();
    }
    inline const_iterator end() const {
        return cards.end();
    }
    inline unsigned getSeed() const {
        return seed;
    }
};

class CardPile {
private:
    uint8_t internalSize;
    uint8_t numHidden;
    Card* pile;
public:
    CardPile() : internalSize(0), numHidden(0), pile(nullptr) {}
    CardPile(uint_fast8_t numCards, uint_fast8_t numHidden) : internalSize(numCards), numHidden(numHidden), pile(new Card[internalSize]) {
        if(numHidden > internalSize) {
            numHidden = internalSize;
        }
    };
    CardPile(const CardPile& copy) : CardPile(copy.internalSize, copy.numHidden) {
        memcpy(pile, copy.pile, sizeof(Card) * internalSize);
    }
    CardPile(CardPile&& move) : internalSize(move.internalSize), numHidden(move.numHidden), pile(move.pile) {
        move.pile = nullptr;
        move.internalSize = 0;
        move.numHidden = 0;
    }
    ~CardPile() {
        if(pile) {
            delete pile;
        }
    }
    CardPile& operator=(const CardPile& copy) {
        delete pile;
        internalSize = copy.internalSize;
        numHidden = copy.numHidden;
        pile = new Card[internalSize];
        memcpy(pile, copy.pile, sizeof(Card) * internalSize);
        return *this;
    }
    CardPile& operator=(CardPile&& move) {
        delete pile;
        internalSize = move.internalSize;
        numHidden = move.numHidden;
        pile = move.pile;
        move.pile = nullptr;
        move.internalSize = 0;
        move.numHidden = 0;
        return *this;
    }
    inline size_t size() const { return internalSize; }
    inline bool empty() const { return size() == 0; }
    inline size_t getNumHidden() const { return numHidden; }
    inline Card operator[](size_t index) const {
        if(index < numHidden) {
            return Card();
        } else if(index >= internalSize) {
            return Card::EMPTY;
        } else {
            return pile[index];
        }
    }
    inline Card& operator[](size_t index) {
        if(index < numHidden) {
            return Card::UNKNOWN;
        } else if(index >= internalSize) {
            return Card::EMPTY;
        } else {
            return pile[index];
        }
    }
    CardPile addTop(Card newCard) const {
        CardPile ret(internalSize + 1, numHidden);
        memcpy(ret.pile, pile, sizeof(Card) * internalSize);
        ret.pile[internalSize] = newCard;
        return ret;
    }
    CardPile removeTop() const {
        CardPile ret(internalSize - 1, numHidden);
        memcpy(ret.pile, pile, sizeof(Card) * (internalSize - 1));
        return ret;
    }
    inline Card top() const {
        if(empty()) {
            return Card::EMPTY;
        } else {
            return (*this)[internalSize - 1];
        }
    }
};

class GameState {
private:
    CardPile stockPile;
    CardPile waste;
    CardPile tableaus[7];
    CardPile foundations[4];
public:
    GameState(const Deck& deck) : stockPile(23, 23), waste(1, 0) {
        size_t deckOffset = 0;
        for(size_t i=0; i<7; ++i) {
            tableaus[i] = CardPile(i + 1, i);
            for(size_t j=0; j<=i; ++j) {
                tableaus[i][j] = deck[deckOffset++];
            }
        }
        waste[0] = deck[deckOffset++];
        for(size_t i=0; i<stockPile.size(); ++i) {
            stockPile[i] = deck[deckOffset++];
        }
    }
    const CardPile& getStockPile() const { return stockPile; }
    const CardPile& getWaste() const { return waste; }
    const CardPile& getTableau(uint_fast8_t index) const { return tableaus[index]; }
    const CardPile& getFoundation(uint_fast8_t index) const { return foundations[index]; }
};

std::ostream& operator<<(std::ostream& stream, const GameState& state) {
    stream << (state.getStockPile().empty() ? "--" : "[]") << " " << state.getWaste().top() << "   ";
    for(size_t i=0; i<4; ++i) {
        stream << " " << state.getFoundation(i).top();
    }
    stream << std::endl << std::endl;
    bool allEmpty = false;
    for(size_t row=0; !allEmpty; ++row) {
        std::stringstream ss;
        allEmpty = true;
        for(size_t i=0; i<7; ++i) {
            Card c = state.getTableau(i)[row];
            if(i > 0) {
                ss << " ";
            }
            if(c == Card::EMPTY) {
                ss << "  ";
            } else {
                allEmpty = false;
                ss << c;
            }
        }
        if(!allEmpty) {
            stream << ss.str() << std::endl;
        }
    }
    
    return stream;
}

int main(int, char**) {
    Deck deck;
    GameState game(deck);
    std::cout << "Game #" << deck.getSeed() << std::endl << std::endl;
    std::cout << game;
}
