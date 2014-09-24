/*
 Copyright (C) 2010-2014 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "CollectionUtils.h"
#include "Model/Node.h"
#include "Model/NodeVisitor.h"

namespace TrenchBroom {
    namespace Model {
        class MockNode : public Node {
        private: // implement Node interface
            Node* doClone(const BBox3& worldBounds) const {
                return new MockNode();
            }
            
            bool doCanAddChild(const Node* child) const {
                return mockDoCanAddChild(child);
            }
            
            bool doCanRemoveChild(const Node* child) const {
                return mockDoCanRemoveChild(child);
            }
            
            void doParentWillChange() {
                mockDoParentWillChange();
            }
            
            void doParentDidChange() {
                mockDoParentDidChange();
            }

            bool doSelectable() const {
                return mockDoSelectable();
            }
            
            void doAncestorWillChange() {
                mockDoAncestorWillChange();
            }
            
            void doAncestorDidChange() {
                mockDoAncestorDidChange();
            }
            
            void doAccept(NodeVisitor& visitor) {
                mockDoAccept(visitor);
            }
            
            void doAccept(ConstNodeVisitor& visitor) const {
                mockDoAccept(visitor);
            }
        public:
            MOCK_CONST_METHOD1(mockDoCanAddChild, bool(const Node*));
            MOCK_CONST_METHOD1(mockDoCanRemoveChild, bool(const Node*));
            MOCK_CONST_METHOD0(mockDoSelectable, bool());
            
            MOCK_METHOD0(mockDoParentWillChange, void());
            MOCK_METHOD0(mockDoParentDidChange, void());
            MOCK_METHOD0(mockDoAncestorWillChange, void());
            MOCK_METHOD0(mockDoAncestorDidChange, void());
            
            MOCK_METHOD1(mockDoAccept, void(NodeVisitor&));
            MOCK_CONST_METHOD1(mockDoAccept, void(ConstNodeVisitor&));
        };
        
        class TestNode : public Node {
        private: // implement Node interface
            virtual Node* doClone(const BBox3& worldBounds) const {
                return new TestNode();
            }
            
            virtual bool doCanAddChild(const Node* child) const {
                return true;
            }
            
            virtual bool doCanRemoveChild(const Node* child) const {
                return true;
            }
            
            virtual bool doSelectable() const {
                return true;
            }
            
            virtual void doParentWillChange() {}
            virtual void doParentDidChange() {}
            virtual void doAncestorWillChange() {}
            virtual void doAncestorDidChange() {}
            virtual void doAccept(NodeVisitor& visitor) {}
            virtual void doAccept(ConstNodeVisitor& visitor) const {}
        };
        
        class DestroyableNode : public TestNode {
        private:
            bool& m_destroyed;
        public:
            DestroyableNode(bool& destroyed) :
            m_destroyed(destroyed) {}
            
            ~DestroyableNode() {
                m_destroyed = true;
            }
        };
        
        TEST(NodeTest, destroyChild) {
            using namespace ::testing;
            
            TestNode* root = new TestNode();
            bool childDestroyed = false;
            DestroyableNode* child = new DestroyableNode(childDestroyed);
            
            root->addChild(child);
            delete root;
            
            ASSERT_TRUE(childDestroyed);
        }
        
        TEST(NodeTest, addRemoveChild) {
            using namespace ::testing;
            
            MockNode root;
            MockNode* child = new MockNode();
            MockNode* grandChild1 = new MockNode();
            MockNode* grandChild2 = new MockNode();
            
            EXPECT_CALL(*child, mockDoCanAddChild(grandChild1)).WillOnce(Return(true));
            EXPECT_CALL(*grandChild1, mockDoParentWillChange());
            EXPECT_CALL(*grandChild1, mockDoAncestorWillChange());
            EXPECT_CALL(*grandChild1, mockDoParentDidChange());
            EXPECT_CALL(*grandChild1, mockDoAncestorDidChange());
            child->addChild(grandChild1);
            ASSERT_EQ(1u, child->childCount());
            ASSERT_EQ(2u, child->familySize());
            ASSERT_EQ(child, grandChild1->parent());
            ASSERT_TRUE(VectorUtils::contains(child->children(), grandChild1));
            
            EXPECT_CALL(root, mockDoCanAddChild(child)).WillOnce(Return(true));
            EXPECT_CALL(*child, mockDoParentWillChange());
            EXPECT_CALL(*child, mockDoAncestorWillChange());
            EXPECT_CALL(*child, mockDoParentDidChange());
            EXPECT_CALL(*child, mockDoAncestorDidChange());
            EXPECT_CALL(*grandChild1, mockDoAncestorWillChange());
            EXPECT_CALL(*grandChild1, mockDoAncestorDidChange());
            
            root.addChild(child);
            ASSERT_EQ(1u, root.childCount());
            ASSERT_EQ(3u, root.familySize());
            ASSERT_EQ(&root, child->parent());
            ASSERT_TRUE(VectorUtils::contains(root.children(), child));
            
            EXPECT_CALL(*child, mockDoCanAddChild(grandChild2)).WillOnce(Return(true));
            EXPECT_CALL(*grandChild2, mockDoParentWillChange());
            EXPECT_CALL(*grandChild2, mockDoAncestorWillChange());
            EXPECT_CALL(*grandChild2, mockDoParentDidChange());
            EXPECT_CALL(*grandChild2, mockDoAncestorDidChange());
            child->addChild(grandChild2);
            ASSERT_EQ(1u, root.childCount());
            ASSERT_EQ(4u, root.familySize());
            ASSERT_EQ(2u, child->childCount());
            ASSERT_EQ(3u, child->familySize());
            ASSERT_EQ(child, grandChild2->parent());
            ASSERT_TRUE(VectorUtils::contains(child->children(), grandChild2));
            
            EXPECT_CALL(root, mockDoCanRemoveChild(child)).WillOnce(Return(true));
            EXPECT_CALL(*child, mockDoParentWillChange());
            EXPECT_CALL(*child, mockDoAncestorWillChange());
            EXPECT_CALL(*child, mockDoParentDidChange());
            EXPECT_CALL(*child, mockDoAncestorDidChange());
            EXPECT_CALL(*grandChild1, mockDoAncestorWillChange());
            EXPECT_CALL(*grandChild1, mockDoAncestorDidChange());
            EXPECT_CALL(*grandChild2, mockDoAncestorWillChange());
            EXPECT_CALL(*grandChild2, mockDoAncestorDidChange());

            root.removeChild(child);
            ASSERT_EQ(NULL, child->parent());
            ASSERT_FALSE(VectorUtils::contains(root.children(), child));
            ASSERT_EQ(0u, root.childCount());
            ASSERT_EQ(1u, root.familySize());
            ASSERT_EQ(2u, child->childCount());
            ASSERT_EQ(3u, child->familySize());
            
            EXPECT_CALL(root, mockDoCanAddChild(child)).WillOnce(Return(true));
            EXPECT_CALL(*child, mockDoParentWillChange());
            EXPECT_CALL(*child, mockDoAncestorWillChange());
            EXPECT_CALL(*child, mockDoParentDidChange());
            EXPECT_CALL(*child, mockDoAncestorDidChange());
            EXPECT_CALL(*grandChild1, mockDoAncestorWillChange());
            EXPECT_CALL(*grandChild1, mockDoAncestorDidChange());
            EXPECT_CALL(*grandChild2, mockDoAncestorWillChange());
            EXPECT_CALL(*grandChild2, mockDoAncestorDidChange());
            
            root.addChild(child);
            ASSERT_EQ(&root, child->parent());
            ASSERT_TRUE(VectorUtils::contains(root.children(), child));
            ASSERT_EQ(1u, root.childCount());
            ASSERT_EQ(4u, root.familySize());
            ASSERT_EQ(2u, child->childCount());
            ASSERT_EQ(3u, child->familySize());
        }
        
        TEST(NodeTest, partialSelection) {
            TestNode root;
            TestNode* child1 = new TestNode();
            TestNode* child2 = new TestNode();
            TestNode* grandChild1_1 = new TestNode();
            TestNode* grandChild1_2 = new TestNode();
            
            root.addChild(child1);
            root.addChild(child2);
            
            ASSERT_EQ(0u, root.familyMemberSelectionCount());
            child1->select();
            ASSERT_EQ(1u, child1->familyMemberSelectionCount());
            ASSERT_EQ(1u, root.familyMemberSelectionCount());
            child2->select();
            ASSERT_EQ(1u, child2->familyMemberSelectionCount());
            ASSERT_EQ(2u, root.familyMemberSelectionCount());
            
            child1->deselect();
            ASSERT_EQ(0u, child1->familyMemberSelectionCount());
            ASSERT_EQ(1u, root.familyMemberSelectionCount());

            grandChild1_1->select();
            child1->addChild(grandChild1_1);
            ASSERT_EQ(1u, child1->familyMemberSelectionCount());
            ASSERT_EQ(2u, root.familyMemberSelectionCount());

            child1->addChild(grandChild1_2);
            ASSERT_EQ(1u, child1->familyMemberSelectionCount());
            ASSERT_EQ(2u, root.familyMemberSelectionCount());
            grandChild1_2->select();
            ASSERT_EQ(2u, child1->familyMemberSelectionCount());
            ASSERT_EQ(3u, root.familyMemberSelectionCount());

            grandChild1_1->deselect();
            ASSERT_EQ(1u, child1->familyMemberSelectionCount());
            ASSERT_EQ(2u, root.familyMemberSelectionCount());
        }
    }
}