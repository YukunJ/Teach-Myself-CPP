#include "../src/skiplist.h"

#include <gtest/gtest.h>

namespace kvstore {
TEST(SkipNodeTest, SkipNodeLinkage) {
  // test if the SkipNode's linkage is functioning correctly

  /*
   create a linkage like:
        (6, 20)
           |
  -oo - (2,4) - +00
   */
  auto handle_start = new SkipNode<int, int>(0, 0, true);
  auto handle_end = new SkipNode<int, int>(0, 0, true);
  auto center = new SkipNode<int, int>(2, 4);
  handle_start->SetAfter(center);
  handle_end->SetBefore(center);
  center->SetBefore(handle_start);
  center->SetAfter(handle_end);
  center->SetAbove(new SkipNode<int, int>(6, 20));
  center->GetAbove()->SetBelow(center);

  EXPECT_EQ(center->GetKey(), 2);
  EXPECT_EQ(center->GetValue(), 4);
  EXPECT_EQ(center->GetAbove()->GetKey(), 6);
  EXPECT_EQ(center->GetBelow(), nullptr);

  // clean up
  free(center->GetBefore());
  free(center->GetAfter());
  free(center->GetAbove());
  free(center);
}

TEST(SkipNodeTest, SkipNodeSkipSearch) {
  // test if the SkipNode's SkipSearch is working correctly
  // find the biggest node of key that's smaller or equal to given key

  // create such a topology of SkipNode linkages
  /*
    -oo <-> 2 <-------------------------> 9 <-> +oo
     |      |                             |       |
    -oo <-> 2 <---------5 <-> 7---------> 9 <-> +oo
     |      |           |     |           |       |
    -oo <-> 2 <-> 4 <-> 5 <-> 7---------> 9 <-> +oo
  */
  auto head_1 = new SkipNode<int, int>(0, 0, true);
  auto head_2 = new SkipNode<int, int>(0, 0, true);
  auto head_3 = new SkipNode<int, int>(0, 0, true);

  auto tail_1 = new SkipNode<int, int>(0, 0, true);
  auto tail_2 = new SkipNode<int, int>(0, 0, true);
  auto tail_3 = new SkipNode<int, int>(0, 0, true);

  auto node_2_1 = new SkipNode<int, int>(2, 0);
  auto node_2_2 = new SkipNode<int, int>(2, 0);
  auto node_2_3 = new SkipNode<int, int>(2, 0);

  auto node_9_1 = new SkipNode<int, int>(9, 0);
  auto node_9_2 = new SkipNode<int, int>(9, 0);
  auto node_9_3 = new SkipNode<int, int>(9, 0);

  auto node_5_2 = new SkipNode<int, int>(5, 0);
  auto node_5_3 = new SkipNode<int, int>(5, 0);

  auto node_7_2 = new SkipNode<int, int>(7, 0);
  auto node_7_3 = new SkipNode<int, int>(7, 0);

  auto node_4_3 = new SkipNode<int, int>(4, 0);

  // we only link "after" and "below" linkage here for convenience
  // construct first layer
  head_1->SetAfter(node_2_1);
  node_2_1->SetAfter(node_9_1);
  node_9_1->SetAfter(tail_1);

  // construct second layer
  head_2->SetAfter(node_2_2);
  node_2_2->SetAfter(node_5_2);
  node_5_2->SetAfter(node_7_2);
  node_7_2->SetAfter(node_9_2);
  node_9_2->SetAfter(tail_2);

  // construct third layer
  head_3->SetAfter(node_2_3);
  node_2_3->SetAfter(node_4_3);
  node_4_3->SetAfter(node_5_3);
  node_5_3->SetAfter(node_7_3);
  node_7_3->SetAfter(node_9_3);
  node_9_3->SetAfter(tail_3);

  // link first and second layer
  head_1->SetBelow(head_2);
  node_2_1->SetBelow(node_2_2);
  node_9_1->SetBelow(node_9_2);
  tail_1->SetBelow(tail_2);

  // link second and third layer
  head_2->SetBelow(head_3);
  node_2_2->SetBelow(node_2_3);
  node_5_2->SetBelow(node_5_3);
  node_7_2->SetBelow(node_7_3);
  node_9_2->SetBelow(node_9_3);
  tail_2->SetBelow(tail_3);

  auto ans1 = head_1->SkipSearch(6).first;
  EXPECT_EQ(ans1->GetKey(), 5);

  auto ans2 = head_1->SkipSearch(5).first;
  EXPECT_EQ(ans2->GetKey(), 5);

  auto ans3 = head_1->SkipSearch(10).first;
  EXPECT_EQ(ans3->GetKey(), 9);

  auto ans4 = head_1->SkipSearch(3).first;
  EXPECT_EQ(ans4->GetKey(), 2);

  auto ans5 = head_1->SkipSearch(1).first;
  EXPECT_EQ(ans5->IsSentinel(), true);

  // clean up
  delete head_1;
  delete head_2;
  delete head_3;
  delete tail_1;
  delete tail_2;
  delete tail_3;
  delete node_2_1;
  delete node_2_2;
  delete node_2_3;
  delete node_9_1;
  delete node_9_2;
  delete node_9_3;
  delete node_5_2;
  delete node_5_3;
  delete node_7_2;
  delete node_7_3;
  delete node_4_3;
}

TEST(SkipListTest, SkipListInsertSearchTest) {
  // test if the SkipList could handle insert and search correctly
  SkipList<int, int> skip;
  skip.SkipInsert(1, 12);
  skip.SkipInsert(4, 13);
  skip.SkipInsert(5, 53);
  skip.SkipInsert(-2, 2);

  auto res1 = skip.SkipSearch(4);
  EXPECT_EQ(res1->GetKey(), 4);
  EXPECT_EQ(res1->GetValue(), 13);

  auto res2 = skip.SkipSearch(0);
  EXPECT_EQ(res2->GetKey(), -2);
  EXPECT_EQ(res2->GetValue(), 2);

  auto res3 = skip.SkipSearch(6);
  EXPECT_EQ(res3->GetKey(), 5);
  EXPECT_EQ(res3->GetValue(), 53);

  // rely on dtor to clean up
}

TEST(SkipListTest, SkipListHeavyInsertTest) {
  SkipList<int, int> skip;
  int test_size = 10000;
  for (int i = 0; i < test_size; i++) {
    skip.SkipInsert(i, i);
  }

  EXPECT_EQ(skip.GetSize(), test_size);

  for (int i = 0; i < test_size; i++) {
    EXPECT_EQ(skip.SkipSearch(i)->GetValue(), i);
  }

  // rely on dtor to clean up
}

TEST(SkipListTest, SkipListSameKeyInsertTest) {
  // test if the SkipList would replace old value when a key is repeated
  // inserted
  SkipList<int, int> skip;
  skip.SkipInsert(1, 12);
  skip.SkipInsert(4, 13);
  skip.SkipInsert(5, 53);
  skip.SkipInsert(-2, 2);

  skip.SkipInsert(-2, 4);
  skip.SkipInsert(-2, 5);
  auto res = skip.SkipSearch(-2);
  EXPECT_EQ(res->GetKey(), -2);
  EXPECT_EQ(res->GetValue(), 5);

  skip.SkipInsert(4, 4);
  skip.SkipInsert(4, 9);
  auto res2 = skip.SkipSearch(4);
  EXPECT_EQ(res2->GetKey(), 4);
  EXPECT_EQ(res2->GetValue(), 9);

  // rely on dtor to clean up
}

TEST(SkipListTest, SkipListRemovalTest) {
  // test if the SkipList could handle delete non-exist key, repeat delete the
  // same key, etc.
  SkipList<int, int> skip;
  skip.SkipInsert(1, 12);
  skip.SkipInsert(4, 13);
  skip.SkipInsert(5, 53);
  skip.SkipInsert(-2, 2);

  EXPECT_EQ(skip.SkipRemove(6), false);
  EXPECT_EQ(skip.SkipRemove(5), true);
  EXPECT_EQ(skip.SkipRemove(5), false);
  EXPECT_NE(skip.SkipSearch(5)->GetKey(), 5);

  // rely on dtor to clean up
}

}  // namespace kvstore