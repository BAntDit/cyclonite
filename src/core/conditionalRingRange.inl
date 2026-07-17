
template<size_t Size, typename ConditionValueType>
template<typename ConditionType>
    requires std::is_nothrow_convertible_v<ConditionValueType, std::decay_t<ConditionType>>
auto ConditionalRingRange<Size, ConditionValueType>::reserveRange(ConditionType&& condition,
                                                                  size_t size,
                                                                  bool forceShiftToBegin /* = false*/) -> size_t
{
    auto offset = RingRange<Size>::reserveRange(size, forceShiftToBegin);
    if (offset != RingRange<Size>::invalid_offset_v) {
        conditions_.emplace_back(std::forward<ConditionType>(condition));
    }
    return offset;
}

template<size_t Size, typename ConditionValueType>
template<typename Pred>
    requires(std::invocable<Pred, ConditionValueType const&> &&
             std::is_same_v<bool, std::invoke_result_t<Pred, ConditionValueType const&>>)
auto ConditionalRingRange<Size, ConditionValueType>::popRange(Pred&& predicate) -> std::pair<size_t, size_t>
{
    auto offset = RingRange<Size>::invalid_offset_v;
    auto size = size_t{ 0 };

    auto&& condition = conditions_.front();
    if (predicate(condition)) {
        conditions_.pop_front();
        auto [ofs, sz] = RingRange<Size>::popRange();
        offset = ofs;
        size = sz;
    }

    return std::pair{ offset, size };
}