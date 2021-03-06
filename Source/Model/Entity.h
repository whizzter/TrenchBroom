/*
 Copyright (C) 2010-2012 Kristian Duske

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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__Entity__
#define __TrenchBroom__Entity__

#include "Model/BrushTypes.h"
#include "Model/EditState.h"
#include "Model/EntityTypes.h"
#include "Model/MapObject.h"
#include "Model/EntityProperty.h"
#include "Utility/Allocator.h"
#include "Utility/VecMath.h"

#include <cstdlib>

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class EntityDefinition;
        class Map;

        class Entity : public MapObject, public Utility::Allocator<Entity> {
        public:
            static String const ClassnameKey;
            static String const NoClassnameValue;
            static String const SpawnFlagsKey;
            static String const WorldspawnClassname;
            static String const GroupClassname;
            static String const GroupNameKey;
            static String const GroupVisibilityKey;
            static String const OriginKey;
            static String const AngleKey;
            static String const AnglesKey;
            static String const MangleKey;
            static String const MessageKey;
            static String const ModKey;
            static String const TargetKey;
            static String const KillTargetKey;
            static String const TargetnameKey;
            static String const WadKey;
            static String const DefKey;
            static String const DefaultDefinition;
            static String const FacePointFormatKey;

            inline static bool isNumberedProperty(const String& pattern, const String& key) {
                if (key.size() < pattern.size())
                    return false;
                for (size_t i = 0; i < pattern.size(); i++)
                    if (key[i] != pattern[i])
                        return false;
                for (size_t i = pattern.size(); i < key.size(); i++)
                    if (key[i] < '0' || key[i] > '9')
                        return false;
                return true;
            }
        protected:
            Map* m_map;
            PropertyStore m_propertyStore;
            BrushList m_brushes;
            bool m_worldspawn;

            EntityDefinition* m_definition;

            unsigned int m_selectedBrushCount;
            unsigned int m_hiddenBrushCount;

            const BBoxf& m_worldBounds;

            mutable BBoxf m_bounds;
            mutable Vec3f m_center;
            mutable bool m_geometryValid;

            EntityList m_linkTargets;
            EntityList m_linkSources;
            EntityList m_killTargets;
            EntityList m_killSources;

            void addLinkTarget(Entity& entity);
            void removeLinkTarget(Entity& entity);
            void addLinkSource(Entity& entity);
            void removeLinkSource(Entity& entity);
            void addKillTarget(Entity& entity);
            void removeKillTarget(Entity& entity);
            void addKillSource(Entity& entity);
            void removeKillSource(Entity& entity);
            
            void addLinkTarget(const PropertyValue& targetname);
            void removeLinkTarget(const PropertyValue& targetname);
            void addKillTarget(const PropertyValue& targetname);
            void removeKillTarget(const PropertyValue& targetname);
            
            void addAllLinkTargets();
            void addAllKillTargets();
            void removeAllLinkTargets();
            void removeAllKillTargets();
            
            void addAllLinkSources(const PropertyValue& targetname);
            void addAllKillSources(const PropertyValue& targetname);
            void removeAllLinkSources();
            void removeAllKillSources();

            void init();
            void validateGeometry() const;

            typedef enum {
                RTNone,
                RTZAngle,
                RTZAngleWithUpDown,
                RTEulerAngles
            } RotationType;

            struct RotationInfo {
                const RotationType type;
                const PropertyKey property;

                RotationInfo(RotationType i_type, const PropertyKey& i_property) :
                type(i_type),
                property(i_property) {}
            };

            const RotationInfo rotationInfo() const;
            void applyRotation(const Mat4f& rotation);
        public:
            Entity(const BBoxf& worldBounds);
            Entity(const BBoxf& worldBounds, const Entity& entityTemplate);
            ~Entity();

            inline MapObject::Type objectType() const {
                return MapObject::EntityObject;
            }

            inline Map* map() const {
                return m_map;
            }

            void setMap(Map* map);

            inline const PropertyList& properties() const {
                return m_propertyStore.properties();
            }

            inline const PropertyValue* propertyForKey(const PropertyKey& key) const {
                return m_propertyStore.propertyValue(key);
            }

            static bool propertyIsMutable(const PropertyKey& key);
            static bool propertyKeyIsMutable(const PropertyKey& key);

            void renameProperty(const PropertyKey& oldKey, const PropertyKey& newKey);
            void removeProperty(const PropertyKey& key);

            void setProperties(const PropertyList& properties, bool replace);
            void setProperty(const PropertyKey& key, const Vec3f& value, bool round);
            void setProperty(const PropertyKey& key, int value);
            void setProperty(const PropertyKey& key, float value, bool round);
            void setProperty(const PropertyKey& key, const PropertyValue& value);
            void setProperty(const PropertyKey& key, const PropertyValue* value);

            StringList linkTargetnames() const;
            StringList killTargetnames() const;

            inline const EntityList& linkTargets() const {
                return m_linkTargets;
            }

            inline const EntityList& linkSources() const {
                return m_linkSources;
            }

            inline const EntityList& killTargets() const {
                return m_killTargets;
            }

            inline const EntityList& killSources() const {
                return m_killSources;
            }

            inline const PropertyValue* classname() const {
                return propertyForKey(ClassnameKey);
            }
            
            inline const PropertyValue& safeClassname() const {
                const PropertyValue* classn = classname();
                if (classn != NULL)
                    return *classn;
                return NoClassnameValue;
            }

            inline bool worldspawn() const {
                return m_worldspawn;
            }

            inline const Vec3f origin() const {
                const PropertyValue* value = propertyForKey(OriginKey);
                if (value == NULL)
                    return Vec3f::Null;
                return Vec3f(*value);
            }

            inline bool rotated() const {
                if (classname() == NULL)
                    return false;
                if (Utility::startsWith(*classname(), "light")) {
                    if (propertyForKey(MangleKey) != NULL)
                        return true;
                } else {
                    if (propertyForKey(AngleKey) != NULL)
                        return true;
                    if (propertyForKey(AnglesKey) != NULL)
                        return true;
                }
                return false;
            }

            const Quatf rotation() const;

            inline const BrushList& brushes() const {
                return m_brushes;
            }

            void addBrush(Brush& brush);
            void addBrushes(const BrushList& brushes);
            void removeBrush(Brush& brush);

            inline EntityDefinition* definition() const {
                return m_definition;
            }

            void setDefinition(EntityDefinition* definition);

            bool selectable() const;

            inline bool partiallySelected() const {
                return m_selectedBrushCount > 0;
            }

            inline void incSelectedBrushCount() {
                m_selectedBrushCount++;
            }

            inline void decSelectedBrushCount() {
                m_selectedBrushCount--;
            }

            inline bool fullyHidden() const {
                return !m_brushes.empty() && m_hiddenBrushCount == m_brushes.size();
            }

            inline void incHiddenBrushCount() {
                m_hiddenBrushCount++;
            }

            inline void decHiddenBrushCount() {
                m_hiddenBrushCount--;
            }

            virtual EditState::Type setEditState(EditState::Type editState);

            inline const BBoxf& worldBounds() const {
                return m_worldBounds;
            }

            inline const Vec3f& center() const {
                if (!m_geometryValid)
                    validateGeometry();
                return m_center;
            }

            inline const BBoxf& bounds() const {
                if (!m_geometryValid)
                    validateGeometry();
                return m_bounds;
            }

            inline void invalidateGeometry() {
                m_geometryValid = false;
            }

            void transform(const Mat4f& pointTransform, const Mat4f& vectorTransform, const bool lockTextures, const bool invertOrientation);
            void pick(const Rayf& ray, PickResult& pickResults);
        };
    }
}

#endif /* defined(__TrenchBroom__Entity__) */
