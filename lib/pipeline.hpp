#ifndef _LABWORK8_LIB_PIPELINE_HPP_
#define _LABWORK8_LIB_PIPELINE_HPP_

#include <functional>
#include <memory>
#include <string>
#include <type_traits>

#include "struct.hpp"

/**
 * Core pipeline abstraction class for lazy data processing chains.
 * 
 * Implements a data processing system where:
 * Data is stored in configurable containers (vector/list/etc)
 * Transformations are applied lazily
 * Operations chain using pipe operator (|)
 * Supports both eager and lazy evaluation models
 * Handles container ownership and memory management automatically
 * 
 * The pipeline operates through 5 key mechanisms:
 * 
 * contains a Data_ inner class managing the actual container
 * Stores computation as std::function until evaluation triggered
 * Evaluation occurs when data is accessed (content()/iterators)
 * Allows building complex transformation chains without intermediate storage
 * 
 * Tracks whether pipeline owns its data via owns_content_ flag
 * Automatically cleans up owned containers on destruction
 * safely shares data between pipeline stages using shared_ptr
 * 
 * Uses RebindS and ContainerTraits for type-safe container transformations
 * Maintains correct allocator types when changing value types
 * Handles container type propagation through SameContainer marker
 * 
 * derive() creates new pipeline stages with transformed data
 * Maintains dependency chain through deriv_invoker callbacks
 * Ensures parent pipelines evaluate before children
 * Propagates const-correctness through container specializations
 * 
 * begin()/end() for range-based for loops
 * Supports both mutable and const iteration
 */
template <
	typename TValue, 
	template <typename, typename...> typename C = std::vector,
	typename ...Args>
class Pipeline {
private:
	//Metafunction that generates a new Pipeline type with a different container
	template <
		typename TValue2, 
		template <typename, typename ...> typename Container2, 
		typename ...Args2>
	struct RebindS{
		using Type = ContainerTraits<Container2, TValue2>::template Resolve<Args2...>;
	};

	// Alias that simplifies accessing the result of RebindS
	template <typename TValue2>
	struct RebindS<TValue2, SameContainer>{
		using Type = ContainerTraits<C, TValue2>::template Resolve<Args...>;
	};

	// CLass that holds the container data and state for lazy evaluation
	class Data_ {
	private:
		// Struct that defines the underlying container used by the pipeline
		template <typename TValue2>
		struct ContainerType{
			using Type = C<TValue, Args...>;
		};

		// Struct that defines the underlying const container used by the pipeline
		template <typename TValue2> 
			requires std::same_as<TValue2, const std::remove_const_t<TValue2>>
		struct ContainerType<TValue2>{
			using Type = const C<std::remove_const_t<TValue>, Args...>;
		};

	public:

		using Type = TValue;
		using Container = ContainerType<TValue>::Type;
		using LazyFunc = std::function<void(Container&)>;

		bool is_alive = true;

		std::function<void()> deriv_invoker = [](){};

		Data_(const LazyFunc& func)
			: content_ptr_(new Container{})
			, lazy_func_ptr_(std::make_unique<LazyFunc>(func))
			, owns_content_(true)
		{}

		Data_(Container& c)
			: content_ptr_(&c)
		{}

		Data_(Container&& c)
			: content_ptr_(new Container(std::move(c)))
			, owns_content_(true)
		{}

		Data_(Data_&) = delete;

		~Data_(){
			if(owns_content_){
				delete content_ptr_;
			}
			is_alive = false;
		}

		void eval() {
			if(lazy_func_ptr_){
				lazy_func_ptr_->operator()(*content_ptr_);
				lazy_func_ptr_.reset();
			}
		}

		Container& content() {
			eval();
			return *content_ptr_;
		}

	private:
		Container* content_ptr_; // pointer to this pipeline's active Data_ storage
		std::unique_ptr<LazyFunc> lazy_func_ptr_; // function to compute the data lazily on demand
		bool owns_content_ = false; // true if this Pipeline instance owns the data container
	};

public:

	using Type = Data_::Type;
	using value_type = Type;
	using Container = Data_::Container;
	using Getter = std::function<Container&()>; // Data accessor


	template <
		typename TValue2, 
		template <typename, typename ...> typename Container2,
		typename ...Args2>
	using Rebind = RebindS<TValue2, Container2, Args2...>::Type;

	// Dduces the Pipeline type when chaining another function lazily
	template <typename NF>
	using NextLazyFunc = std::function<void(Container&, typename NF::Container&)>;

	// Constructs pipeline from an existing container 
	Pipeline(const Data_::LazyFunc& func)
		: data_ptr_(std::make_shared<Data_>(func))
	{}

	// Create an empty pipeline stage (no data yet)
	Pipeline():
		Pipeline(Container{})
	{}

	// Copy constructors for const and regular 
	template <typename Cont>
		requires std::same_as<std::remove_reference_t<Cont>, Container>
	Pipeline(Cont&& c)
		: data_ptr_(std::make_shared<Data_>(std::forward<Cont>(c)))
	{}

	// Regulae opy constructor
	template <typename Cont>
		requires std::same_as<const Cont, Container>
	Pipeline(const Cont& c)
		: data_ptr_(std::make_shared<Data_>(c))
	{}

	const Container& content() const {
		data_ptr_->deriv_invoker();
		return data_ptr_->content();
	}

	std::function<void()> invoker() const {
		auto& data_ptr = data_ptr_;
		return [data_ptr](){
			data_ptr->content();
		};
	}

	Getter CurrentStateGetter() const {
		auto& data_ptr = data_ptr_;
		auto& last_deriv_invoker = data_ptr_->deriv_invoker;

		return [data_ptr, last_deriv_invoker]() -> Container& {
			last_deriv_invoker();
			return data_ptr->content();
		};
	}

	template <typename NF>
	void SetLastDerived(NF nflow) {
		data_ptr_->deriv_invoker = nflow.invoker();
	}

	void SetLastDerived(std::function<void()> invoker) {
		data_ptr_->deriv_invoker = invoker;
	}

	template<
		typename TValue2, 
		template <typename, typename ...> typename Container2, 
		typename ...Args2>
	auto derive(const NextLazyFunc<
			Rebind<TValue2, Container2, 
			Args2...>>& convert) {

		auto getter = CurrentStateGetter();
		
		auto nflow = Rebind<
			TValue2, 
			Container2, 
			Args2...>([convert, getter](auto& ncont){
			convert(getter(), ncont);
		});

		SetLastDerived(nflow);

		return nflow;
	}

	// Applies .apply method to all the adapters that were used via operator |
	// Adapters must have apply() method for pipeline to work
	template <typename Adapter>
	auto operator|(Adapter&& adapter){
		return adapter.apply(*this);
	}

	auto begin(){
		return content().begin();
	}

	auto end(){
		return content().end();
	}

	auto begin() const{
		return content().cbegin();
	}

	auto end() const{
		return content().cend();
	}
private:
	std::shared_ptr<Data_> data_ptr_;
};

#endif // _LABWORK8_LIB_PIPELINE_HPP_