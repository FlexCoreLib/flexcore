namespace fc
{
namespace tests
{

///A data structure for counting calls to various constructors.
struct constructor_count
{
	int times_constructed;
	int times_moved;
	int times_copied;
};

/**
 * a copyable and movable connectable that keeps track of the times it has been copied/moved. The
 * counts are written to *ptr when operator() is called.
 */
struct movable_connectable
{
	explicit movable_connectable(constructor_count* ptr) : count_{1, 0, 0}, ptr(ptr) {}
	movable_connectable(const movable_connectable& other) : count_(other.count_), ptr(other.ptr)
	{
		++count_.times_copied;
	}
	movable_connectable(movable_connectable&& other) : count_(other.count_), ptr(other.ptr)
	{
		++count_.times_moved;
	}

	movable_connectable& operator=(movable_connectable&& o) = delete;
	movable_connectable& operator=(const movable_connectable& o) = delete;

	int operator()(int value)
	{
		*ptr = count_;
		return value + 1;
	}

	constructor_count count_;
	constructor_count* ptr;
};

}
}
