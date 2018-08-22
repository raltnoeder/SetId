#include <GroupIdList.h>

// @throws std::bad_alloc
GroupIdList::GroupIdList()
{
    gid_list_mgr = std::unique_ptr<VList<gid_t>>(new VList<gid_t>(&dsaext::generic_compare<gid_t>));
    gid_list = gid_list_mgr.get();
}

GroupIdList::~GroupIdList() noexcept
{
    VList<gid_t>::ValuesIterator iter(*gid_list);
    while (iter.has_next())
    {
        delete iter.next();
    }
    gid_list->clear();
}

// @throws std::bad_alloc
void GroupIdList::add_entry(gid_t group_id)
{
    std::unique_ptr<gid_t> entry_mgr(new gid_t {group_id});
    gid_list->append(entry_mgr.get());
    entry_mgr.release();
}

size_t GroupIdList::get_size() noexcept
{
    return gid_list->get_size();
}

// @throws std::bad_alloc
gid_t* GroupIdList::get_array()
{
    size_t gid_list_size = gid_list->get_size();
    gid_t* gid_array = new gid_t[gid_list_size];
    VList<gid_t>::ValuesIterator iter(*gid_list);

    for (size_t idx = 0; idx < gid_list_size; ++idx)
    {
        gid_array[idx] = *iter.next();
    }

    return gid_array;
}
