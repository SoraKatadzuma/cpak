#pragma once
#include "common.hpp"
#include "yaml-cpp/yaml.h"


namespace cpak {


/// @brief   Specialization for \c Properties which contain a single value.
/// @tparam  TContained The type of data contained in the property.
/// @details This class is used to store a single value, however, that value
///          can be marked as required. Required values must be provided by
///          the user when the this property is constructed.
template<typename TContained, bool required>
struct Property {
    Property()  = default;
    ~Property() = default;

    /// @brief Creates a new \c Property from the given \c Property.
    /// @param rhs The \c Property to copy into this property.
    Property(const Property& rhs)
        : value_(rhs.value_) {}

    /// @brief Creates a new \c Property from the given \c Property.
    /// @param rhs The \c Property to move into this property.
    Property(Property&& rhs)
        : value_(std::move(rhs.value_)) {}

    /// @brief Creates a new \c Property from the given value.
    /// @param value The value to copy into this property.
    Property(const TContained& value)
        : value_(value) {}
    
    /// @brief Creates a new \c Property from the given value.
    /// @param value The value to move into this property.
    Property(TContained&& value)
        : value_(std::move(value)) {}

    /// @brief  Assigns the given \c Property to this \c Property.
    /// @param  rhs The property to assign the value from.
    /// @return The assigned property.
    Property& operator=(const Property& rhs) {
        value_ = rhs.value_;
        return *this;
    }

    /// @brief  Assigns the given \c Property to this \c Property.
    /// @param  rhs The property to assign the value from.
    /// @return The assigned property.
    Property& operator=(Property&& rhs) {
        value_ = std::move(rhs.value_);
        return *this;
    }

    /// @brief  Assigns nothing to this \c Property.
    /// @return 
    Property& operator=(std::nullopt_t) {
        // Make sure we're not assigning nothing to a property which requires
        // a value to be present. This could cause problems for serialization.
        assert(required && "Cannot assign std::nullopt to required property.");

        value_ = std::nullopt;
        return *this;
    }

    /// @brief  Compares this \c Property to the given \c Property.
    /// @param  rhs The property to compare to.
    /// @return \c true if the properties are equal, \c false otherwise.
    bool operator==(const Property& rhs) const noexcept {
        return value_ == rhs.value_;
    }

    /// @brief  Compares this \c Property to a null optional.
    /// @return \c true if this \c Property has no value, \c false otherwise.
    bool operator==(std::nullopt_t) const noexcept {
        return !value_.has_value();
    }

    /// @brief  Compares this \c Property to the given \c Property.
    /// @param  rhs The property to compare to.
    /// @return \c true if the properties are not equal, \c false otherwise.
    bool operator!=(const Property& rhs) const noexcept {
        return !(value_ == rhs.value_);
    }

    /// @brief  Compares this \c Property to a null optional.
    /// @return \c true if this \c Property has a value, \c false otherwise.
    bool operator!=(std::nullopt_t) const noexcept {
        return value_.has_value();
    }

    /// @brief  Gets the value of this \c Property.
    /// @return The value of this \c Property.
    TContained& operator*() noexcept {
        return value_.value();
    }

    /// @brief  Provides access to the value of this \c Property.
    /// @return A reference to the value of this \c Property.
    /// @throws std::runtime_error if this property is required and has no value.
    TContained* operator->() {
        // Make sure we're not accessing a required property which has no value.
        assert(!(required && !value_.has_value()) && "Required property has no value.");
        return value_.operator->();
    }

    /// @brief  Gets the value of this \c Property.
    /// @return The value of this \c Property.
    const TContained& operator*() const noexcept {
        return value_.value();
    }

    /// @brief  Provides access to the value of this \c Property.
    /// @return A reference to the value of this \c Property.
    /// @throws std::runtime_error if this property is required and has no value.
    const TContained* operator->() const {
        // Make sure we're not accessing a required property which has no value.
        assert(!(required && !value_.has_value()) && "Required property has no value.");
        return value_.operator->();
    }

    /// @brief  Checks whether or not this \c Property has a value.
    /// @return \c true if this \c Property has a value, \c false otherwise.
    bool has_value() const noexcept {
        return value_.has_value();
    }

private:
    std::optional<TContained> value_;
};



/// @brief   Specialization for Properties which store a vector of values.
/// @tparam  TContained The type of the data contained in the vector.
/// @details This class is used to store an insertion ordered list of values
///          which do are not duplicated. Essentially it's a chronologically
///          ordered set.
/// @remark  Vector properties do not require values as they can be empty. It
///          is up to the user if a vector property is required or not. That
///          is, if the property is required to have at least one value.
template<typename TContained>
struct Property<std::vector<TContained>, false> {
    Property()  = default;
    ~Property() = default;

    /// @brief Creates a new \c Property from the given \c Property.
    /// @param rhs The \c Property to copy into this property.
    Property(const Property& rhs)
        : value_(rhs.value_) {}

    /// @brief Creates a new \c Property from the given \c Property.
    /// @param rhs The \c Property to move into this property.
    Property(Property&& rhs)
        : value_(std::move(rhs.value_)) {}

    /// @brief Creates a new \c Property from the given vector.
    /// @param value The vector to copy into this property.
    Property(const std::vector<TContained>& value)
        : value_(value) {}

    /// @brief Creates a new \c Property from the given vector.
    /// @param value The vector to move into this property.
    Property(std::vector<TContained>&& value)
        : value_(std::move(value)) {}

    /// @brief  Assigns the given \c Property to this \c Property.
    /// @param  rhs The property to assign values from.
    /// @return The assigned property.
    Property& operator=(const Property& rhs) {
        value_ = rhs.value_;
        return *this;
    }

    /// @brief  Assigns the given \c Property to this \c Property.
    /// @param  rhs The property to assign values from.
    /// @return The assigned property.
    Property& operator=(Property&& rhs) {
        value_ = std::move(rhs.value_);
        return *this;
    }

    /// @brief   Assigns the null optional to the property.
    /// @return  The assigned property.
    /// @remarks This will clear the property of all values, just like it would
    ///          for the single value 'optional' property.
    Property& operator=(std::nullopt_t) {
        value_.clear();
        return *this;
    }

    /// @brief  Compares this \c Property to the given \c Property.
    /// @param  rhs The property to compare to.
    /// @return \c true if the properties are equal, \c false otherwise.
    bool operator==(const Property& rhs) const noexcept {
        return value_ == rhs.value_;
    }

    /// @brief  Compares this \c Property to a null optional.
    /// @return \c true if this \c Property has no value, \c false otherwise.
    bool operator==(std::nullopt_t) const noexcept {
        return !value_.has_value();
    }

    /// @brief  Compares this \c Property to the given \c Property.
    /// @param  rhs The property to compare to.
    /// @return \c true if the properties are not equal, \c false otherwise.
    bool operator!=(const Property& rhs) const noexcept {
        return !(*this == rhs);
    }

    /// @brief  Compares this \c Property to a null optional.
    /// @return \c true if this \c Property has a value, \c false otherwise.
    bool operator!=(std::nullopt_t) const noexcept {
        return value_.size() > 0;
    }

    /// @brief  Gets the value of this \c Property.
    /// @return The value of this \c Property.
    std::vector<TContained>& operator*() noexcept {
        return value_;
    }

    /// @brief  Provides access to the value of this \c Property.
    /// @return The value of this \c Property.
    std::vector<TContained>* operator->() noexcept {
        return &value_;
    }

    /// @brief  Gets the value of this \c Property.
    /// @return The value of this \c Property.
    const std::vector<TContained>& operator*() const noexcept {
        return value_;
    }

    /// @brief  Provides access to the value of this \c Property.
    /// @return The value of this \c Property.
    const std::vector<TContained>* operator->() const noexcept {
        return &value_;
    }

    /// @brief   Pipes the given \c Property into this \c Property.
    /// @param   rhs The property to pipe into this one.
    /// @return  Th newly created property.
    Property operator|(const Property& rhs) noexcept {
        Property result = *this;
        std::copy_if(
            rhs.value_.begin(),
            rhs.value_.end(),
            std::back_inserter(result.value_),
            [this](const auto& value) {
                return std::find(value_.begin(), value_.end(), value) == value_.end();
            }
        );

        return result;
    }

    /// @brief   Pipes the given \c Property into this \c Property.
    /// @param   rhs The property to pipe into this one.
    /// @return  This updated property.
    /// @remarks This will append unique values from the given property into
    ///          this property. It's treated like a set to prevent duplicates.
    ///          This operation is very specific to the way that \c BuildTargets
    ///          will be used.
    Property& operator|=(const Property& rhs) noexcept {
        std::copy_if(
            rhs.value_.begin(),
            rhs.value_.end(),
            std::back_inserter(value_),
            [this](const auto& value) {
                return std::find(value_.begin(), value_.end(), value) == value_.end();
            }
        );

        return *this;
    }

    /// @brief  Checks whether or not this \c Property has a value.
    /// @return \c true if this \c Property has a value, \c false otherwise.
    bool has_value() const noexcept {
        return value_.size() > 0;
    }

private:
    std::vector<TContained> value_;
};


template<typename TContained>
using RequiredProperty = Property<TContained, true>;

template<typename TContained>
using OptionalProperty = Property<TContained, false>;

template<typename TContained>
using VectorProperty = Property<std::vector<TContained>, false>;


/// @brief  Creates a vector property from a string.
/// @param  value The string to create the property from.
/// @param  delimiter How to split the string into values.
/// @return The created property.
inline VectorProperty<std::string>
vectorPropertyFromString(std::string_view value, char delimiter = ' ') noexcept {
    // Split the string into vector of strings, then assign.
    std::vector<std::string> values;
    std::istringstream iss(value.data());
    std::string token;

    while (std::getline(iss, token, delimiter))
        values.push_back(token);

    return Property<std::vector<std::string>, false>(std::move(values));
}


/// @brief  Creates a string from a vector property.
/// @param  property The property to create the string from.
/// @param  delimiter How to join the values into a string.
/// @return The created string.
inline std::string
vectorPropertyToString(const VectorProperty<std::string>& property, char delimiter = ' ') noexcept {
    std::ostringstream oss;
    for (const auto& value : *property)
        oss << value << delimiter;
    return oss.str();
}


}


template<>
struct YAML::convert<cpak::RequiredProperty<std::string>> {
    static Node encode(const cpak::RequiredProperty<std::string>& rhs) {
        return Node(*rhs);
    }

    static bool decode(const Node& node, cpak::RequiredProperty<std::string>& rhs) {
        rhs = node.as<std::string>();
        return true;
    }
};

template<>
struct YAML::convert<cpak::OptionalProperty<std::string>> {
    static Node encode(const cpak::OptionalProperty<std::string>& rhs) {
        return rhs.has_value()
            ? Node(*rhs)
            : Node();
    }

    static bool decode(const Node& node, cpak::OptionalProperty<std::string>& rhs) {
        rhs = node.as<std::string>();
        return true;
    }
};

template<>
struct YAML::convert<cpak::VectorProperty<std::string>> {
    static Node encode(const cpak::VectorProperty<std::string>& rhs) {
        return rhs.has_value()
            ? Node(*rhs)
            : Node();
    }

    static bool decode(const Node& node, cpak::VectorProperty<std::string>& rhs) {
        rhs = node.as<std::vector<std::string>>();
        return true;
    }
};