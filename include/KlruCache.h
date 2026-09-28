#ifndef KLRUCACHE_H
#define KLRUCACHE_H
#include<iostream>
#include<cstring>
#include<mutex>
#include<memory>
#include<unordered_map>
#include "KICachePolicy.h"

template<typename Key,typename Value> class KLruCache;

template<typename Key,typename Value>
class LruNode
{
private:
    Key key_;   //键
    Value value_;   //值
    size_t accessCount_;    //访问数量
    std::weak_ptr<LruNode> prev_;   //前向节点
    std::shared_ptr<LruNode> next_; //后向节点
public:
    //构造器
    LruNode(Key key,Value value):key_(key),value_(value),accessCount_(1){};

    //取得key
    Key getKey()const{
        return key_;
    }

    //取得value
    Value getValue()const{
        return value_;
    }

    //取得访问数量
    size_t getAccessCount()const{
        return accessCount_;
    }

    //设置value
    void setValue(const Value& value){
        value_=value;
    }

    //
    void incrementAccessCount(){
        ++accessCount_;
    }

    //友元类
    friend class KLruCache<Key,Value>;

};

template<typename Key,typename Value>
class KLruCache:public KICachePolicy <Key,Value>
{
public:
    using LruNodeType = LruNode<Key, Value>;
    using NodePtr = std::shared_ptr<LruNodeType>;
    using NodeMap = std::unordered_map<Key, NodePtr>;

    KLruCache(int capacity):capacity_(capacity){
        initializeList();
    }

    ~KLruCache()override =default;

    void put(Key key,Value value){
        if(capacity_<=0){
            return ;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        auto it=nodeMap_.find(key);
        if(it!=nodeMap_.end()){
            updateExistingNode(it->second,value);
            return ;    
        }
        addNewNode(key,value);
    }

    bool get(Key key,Value& value){
        std::lock_guard<std::mutex> lock(mutex_);
        auto it=nodeMap_.find(key);
        if(it!=nodeMap_.end()){
            moveToMostRecent(it->second);
            value=it->second->getValue();
            return true;
        }else{
            return false;
        }
    }

    void remove(Key key) 
    {   
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = nodeMap_.find(key);
        if (it != nodeMap_.end())
        {
            removeNode(it->second);
            nodeMap_.erase(it);
        }
    }

    
private:
    void initializeList(){
        dummyHead_ = std::make_shared<LruNodeType>(Key(), Value());
        dummyTail_ = std::make_shared<LruNodeType>(Key(), Value());
        dummyHead_->next_ = dummyTail_;
        dummyTail_->prev_ = dummyHead_;
    }

    void updateExistingNode(NodePtr node,const Value& value){
        node->setValue(value);
        moveToMostRecent(node);
    }

    void addNewNode(const Key& key, const Value& value) 
    {
       if (nodeMap_.size() >= capacity_) 
       {
           evictLeastRecent();
       }

       NodePtr newNode = std::make_shared<LruNodeType>(key, value);
       insertNode(newNode);
       nodeMap_[key] = newNode;
    }

    void moveToMostRecent(NodePtr node){
        removeNode(node);
        insertNode(node);
    }

    void removeNode(NodePtr node) 
    {
        if(!node->prev_.expired() && node->next_) 
        {
            auto prev = node->prev_.lock(); // 使用lock()获取shared_ptr
            prev->next_ = node->next_;
            node->next_->prev_ = prev;
            node->next_ = nullptr; // 清空next_指针，彻底断开节点与链表的连接
        }
    }

    void insertNode(NodePtr node) 
    {
        node->next_ = dummyTail_;
        node->prev_ = dummyTail_->prev_;
        dummyTail_->prev_.lock()->next_ = node; // 使用lock()获取shared_ptr
        dummyTail_->prev_ = node;
    }

    void evictLeastRecent() 
    {
        NodePtr leastRecent = dummyHead_->next_;
        removeNode(leastRecent);
        nodeMap_.erase(leastRecent->getKey());
    }
    
private:
    int capacity_;
    NodeMap nodeMap_;
    std::mutex mutex_;
    NodePtr dummyHead_;
    NodePtr dummyTail_;

};
#endif