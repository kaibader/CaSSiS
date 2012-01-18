/*!
 * Tree view
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) GUI.
 *
 * Copyright (C) 2011,2012
 *     Kai Christian Bader <mail@kaibader.de>
 *
 * CaSSiS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * CaSSiS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with CaSSiS.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TREE_H_
#define TREE_H_

#include <QtCore/QString>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsItem>

// Forward declarations...
class Tree;
struct CaSSiSTree;
struct CaSSiSTreeNode;
class NameMap;

class Node: public QGraphicsItem {
    friend class Tree;
public:
    /*!
     * Node class constructor.
     */
    Node(Tree *tree, Node *parent = NULL);

    /*!
     * Node class destructor.
     */
    virtual ~Node();

    /*!
     * Returns the bounding rectangle of the current node.
     */
    QRectF boundingRect() const;

    /*!
     * Returns the shape of the current node.
     */
    QPainterPath shape() const;

    /*!
     * (Re)draws the current node.
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item,
            QWidget *widget);

    /*!
     * Flag. Returns true, if the node is a leaf.
     */
    bool isLeaf() const;

    /*!
     * Sets a whole subtree (in)visible, depending on 'state'.
     */
    void setSubtreeVisibility(bool state);
protected:
    /*!
     * Various mouse event handler...
     */
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
private:
    /*!
     * Pointer to the tree, the node is associated with.
     */
    Tree *m_tree;

    /*!
     * Pointers to the parent, the left and the right child node.
     */
    Node *m_parent, *m_left, *m_right;

    /*!
     * Group or species name that is associated with the node.
     * Unnamed nodes may have a NULL pointer here.
     */
    QString *m_name;

    /*!
     *  A group color. Needed for collapsed child nodes only.
     *  Leafs may have a NULL pointer here.
     */
    QColor *m_color;

    /*
     * A flag that characterizes if a node is collapsed or expanded.
     * true = collapsed
     * false = expanded (default)
     */
    bool m_collapsed;

    /*!
     * Length of the branch between the current node and its parent.
     */
    double m_branch_len;

    /*!
     * Number of species entries in the subtree, including the current node.
     */
    unsigned int m_subtree_species;

    /*!
     * Number of nodes in the subtree, including the current node.
     */
    unsigned int m_subtree_nodes;

    /*!
     * Depth of the current subtree, in nodes.
     */
    unsigned int m_subtree_nodedepth;

    /*!
     * Depth of the current subtree.
     * (i.e. longest path from the current node to a leaf.)
     */
    double m_subtree_depth;

    /*!
     * Depth of the node within the phylogenetic tree.
     * (i.e. number of nodes from the root to this node.)
     */
    unsigned int m_nodedepth;
};

class Tree: public QGraphicsScene {
    friend class Node;Q_OBJECT
public:
    /*!
     * Tree class constructor.
     */
    Tree(const CaSSiSTree *phy_tree, NameMap &map);

    /*!
     * Tree class destructor.
     */
    virtual ~Tree();

    /*!
     * Returns the number of nodes within the tree.
     */
    unsigned int numNodes();

    /*!
     * Returns the number of species within the tree.
     */
    unsigned int numSpecies();

    /*!
     * Returns the depth of the the tree, in nodes.
     */
    unsigned int nodeDepth();

    /*!
     * Fetch a list of identifier strings based on the current selection.
     */
    QStringList selectedIdentifiers();

    /*!
     * Gets the y-distance (spacing) between nodes.
     */
    double yDistance() {
        return m_y_distance;
    }

    /*!
     * Sets the y-distance (spacing) between nodes.
     */
    void setYDistance(double dist) {
        m_y_distance = dist;
    }
public slots:
void treeUpdate();
private:
Node *copy_subtree(Node *parent, CaSSiSTreeNode *cassis_node, NameMap &map,
        unsigned int depth);
double display_subtree(Node *node, double parent_x, double parent_y);
void addSubtreeIdentifiers(Node *node, QStringList &list,
        bool group_selected = false);
unsigned int m_node_counter;
unsigned int m_species_counter;
double m_tree_depth;
unsigned int m_tree_nodedepth;
Node *m_root;
bool m_tree_has_changed;

bool m_autoexpand_unnamed_nodes;

/*!
 * X-Correction factor (Branch length correction factor).
 */
double m_len_correction;

/*!
 * Y-Correction factor (actually 'dist/2' at top and 'dist/2' at the bottom).
 */
double m_y_distance;
};

#endif /* TREE_H_ */
