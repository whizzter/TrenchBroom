/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "EntityLinkIssueGenerator.h"

#include "CollectionUtils.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "Model/Issue.h"
#include "Model/Object.h"
#include "View/ControllerFacade.h"
#include "View/ViewTypes.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        class EntityLinkIssue : public Issue {
        public:
            static const IssueType Type;
        private:
            static const QuickFixType DeleteTargetFix = 0;
            
            Entity* m_entity;
            PropertyKey m_key;
        public:
            EntityLinkIssue(Entity* entity, const PropertyKey& key) :
            Issue(Type),
            m_entity(entity),
            m_key(key) {
                addQuickFix(QuickFix(DeleteTargetFix, Type, "Delete '" + m_key + "' property"));
            }
            
            size_t filePosition() const {
                return m_entity->filePosition();
            }

            String description() const {
                return m_entity->classname() + " entity has missing target for key '" + m_key + "'";
            }
            
            void select(View::ControllerSPtr controller) {
                const BrushList& brushes = m_entity->brushes();
                if (brushes.empty())
                    controller->selectObject(*m_entity);
                else
                    controller->selectObjects(VectorUtils::cast<Model::Object*>(brushes));
            }
            
            void applyQuickFix(const QuickFixType fixType, View::ControllerSPtr controller) {
                controller->removeEntityProperty(EntityList(1, m_entity), m_key);
            }
        private:
            bool doIsHidden(const IssueType type) const {
                return m_entity->isIssueHidden(this);
            }

            void doSetHidden(const IssueType type, const bool hidden) {
                m_entity->setIssueHidden(type, hidden);
            }
        };
        
        const IssueType EntityLinkIssue::Type = Issue::freeType();
        
        IssueType EntityLinkIssueGenerator::type() const {
            return EntityLinkIssue::Type;
        }
        
        const String& EntityLinkIssueGenerator::description() const {
            static const String description("Missing entity link target");
            return description;
        }

        Issue* EntityLinkIssueGenerator::generate(Entity* entity) const {
            assert(entity != NULL);
            const Model::PropertyKeyList missingLinkTargets = entity->findMissingLinkTargets();
            const Model::PropertyKeyList missingKillTargets = entity->findMissingKillTargets();
            
            Issue* issue = NULL;
            processKeys(entity, missingLinkTargets, issue);
            processKeys(entity, missingKillTargets, issue);
            
            return issue;
        }

        void EntityLinkIssueGenerator::processKeys(Entity* entity, const Model::PropertyKeyList& keys, Issue*& issue) const {
            Model::PropertyKeyList::const_iterator it, end;
            for (it = keys.begin(), end = keys.end(); it != end; ++it) {
                const Model::PropertyKey& key = *it;
                Issue* newIssue = new EntityLinkIssue(entity, key);
                if (issue == NULL)
                    issue = newIssue;
                else
                    newIssue->insertAfter(issue);
            }
        }
    }
}