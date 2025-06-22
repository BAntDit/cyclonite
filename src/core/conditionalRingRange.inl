
template<size_t Size, typename ConditionValueType>
template<typename ConditionType>
    requires std::is_same_v<ConditionValueType, std::decay_t<ConditionType>>
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
template<typename ConditionType, typename Pred>
    requires(std::is_same_v<ConditionValueType, std::decay_t<ConditionType>> &&
             std::invocable<Pred, ConditionType &&> &&
             std::is_same_v<bool, std::invoke_result_t<Pred, ConditionType &&>>)
auto ConditionalRingRange<Size, ConditionValueType>::popRange(Pred&& predicate, ConditionType&& condition)
  -> std::pair<size_t, size_t>
{
    auto offset = RingRange<Size>::invalid_offset_v;
    auto size = size_t{ 0 };

    if (predicate(std::forward<ConditionType>(condition))) {
        auto [ofs, sz] = RingRange<Size>::popRange();
        offset = ofs;
        size = sz;
    }

    return std::pair{ offset, size };
}