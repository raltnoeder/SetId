#ifndef GROUIDLIST_H
#define	GROUIDLIST_H

extern "C"
{
    #include <unistd.h>
    #include <sys/types.h>
}

#include <new>
#include <memory>

#include <VList.h>

class GroupIdList
{
  public:
    // @throws std::bad_alloc
    GroupIdList();

    // No copy construction or assignment
    GroupIdList(const GroupIdList& other) = delete;
    GroupIdList(GroupIdList&& orig) = delete;

    virtual GroupIdList& operator=(const GroupIdList& other) = default;
    virtual GroupIdList& operator=(GroupIdList&& other) = default;

    virtual ~GroupIdList() noexcept;

    // @throws std::bad_alloc
    virtual void add_entry(gid_t group_id);

    virtual size_t get_size() noexcept;

    // @throws std::bad_alloc
    virtual gid_t* get_array();

  private:
    std::unique_ptr<VList<gid_t>> gid_list_mgr;
    VList<gid_t>* gid_list;
};

#endif	// GROUIDLIST_H
