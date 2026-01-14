#pragma once

#include <initializer_list>
#include <memory>

const int kBlockSize = 512;
struct Block {
    int buf[kBlockSize / sizeof(int)];
};
class Deque {
private:
    std::unique_ptr<std::unique_ptr<Block>[]> data_;
    std::size_t size_;
    std::size_t capacity_;
    int* first_elem_;
    int* last_elem_;
    std::size_t used_blocks_;
    std::size_t head_block_;
    std::size_t tail_block_;

public:
    void Grow() {
        std::size_t old_cap = capacity_;
        capacity_ *= 2;
        std::unique_ptr<std::unique_ptr<Block>[]> newdata =
            std::make_unique<std::unique_ptr<Block>[]>(capacity_);
        for (std::size_t i = 0; i < used_blocks_; i++) {
            newdata[i] = std::move(data_[(head_block_ + i) % old_cap]);
        }
        data_ = std::move(newdata);
        head_block_ = 0;
        if (used_blocks_ > 0) {
            tail_block_ = used_blocks_ - 1;
        } else {
            tail_block_ = 0;
        }
    }
    Deque() {
        capacity_ = 2;
        data_ = std::make_unique<std::unique_ptr<Block>[]>(capacity_);
        size_ = 0;
        first_elem_ = nullptr;
        last_elem_ = nullptr;
        used_blocks_ = 0;
        head_block_ = 0;
        tail_block_ = 0;
    };

    Deque(const Deque& rhs) {
        capacity_ = rhs.capacity_;
        size_ = rhs.size_;
        first_elem_ = rhs.first_elem_;
        last_elem_ = rhs.last_elem_;
        used_blocks_ = rhs.used_blocks_;
        head_block_ = rhs.head_block_;
        tail_block_ = rhs.tail_block_;
        data_ = std::make_unique<std::unique_ptr<Block>[]>(capacity_);
        for (std::size_t i = 0; i < capacity_; ++i) {
            data_[i] = nullptr;
        }
        for (std::size_t i = 0; i < used_blocks_; ++i) {
            std::size_t ind = (head_block_ + i) % capacity_;
            if (rhs.data_[ind]) {
                data_[ind] = std::make_unique<Block>(*rhs.data_[ind]);
            }
        }
        if (size_ == 0) {
            first_elem_ = last_elem_ = nullptr;
        } else {
            std::size_t head = rhs.first_elem_ - rhs.data_[rhs.head_block_]->buf;
            std::size_t tail = rhs.last_elem_ - rhs.data_[rhs.tail_block_]->buf;
            first_elem_ = &data_[head_block_]->buf[head];
            last_elem_ = &data_[tail_block_]->buf[tail];
        }
    };
    Deque(Deque&& rhs) noexcept {
        capacity_ = rhs.capacity_;
        size_ = rhs.size_;
        first_elem_ = rhs.first_elem_;
        last_elem_ = rhs.last_elem_;
        used_blocks_ = rhs.used_blocks_;
        head_block_ = rhs.head_block_;
        tail_block_ = rhs.tail_block_;
        data_ = std::move(rhs.data_);
        rhs.data_ = std::make_unique<std::unique_ptr<Block>[]>(2);
        rhs.capacity_ = 2;
        rhs.size_ = 0;
        rhs.first_elem_ = nullptr;
        rhs.last_elem_ = nullptr;
        rhs.used_blocks_ = 0;
        rhs.head_block_ = 0;
        rhs.tail_block_ = 0;
    };
    explicit Deque(std::size_t size) {
        const std::size_t b = kBlockSize / sizeof(int);
        if (size == 0) {
            capacity_ = 2;
            data_ = std::make_unique<std::unique_ptr<Block>[]>(capacity_);
            size_ = 0;
            first_elem_ = nullptr;
            last_elem_ = nullptr;
            used_blocks_ = 0;
            head_block_ = 0;
            tail_block_ = 0;
            return;
        }
        std::size_t needed_blocks = (size + b - 1) / b;
        capacity_ = 2;
        while (capacity_ < needed_blocks) {
            capacity_ *= 2;
        }
        data_ = std::make_unique<std::unique_ptr<Block>[]>(capacity_);
        for (std::size_t i = 0; i < capacity_; ++i) {
            data_[i] = nullptr;
        }
        std::size_t all = size;
        for (std::size_t i = 0; i < needed_blocks; ++i) {
            data_[i] = std::make_unique<Block>();
            std::size_t need = all < b ? all : b;
            for (std::size_t j = 0; j < need; ++j) {
                data_[i]->buf[j] = 0;
            }
            all -= need;
        }
        used_blocks_ = needed_blocks;
        head_block_ = 0;
        tail_block_ = used_blocks_ - 1;
        size_ = size;
        first_elem_ = &data_[head_block_]->buf[0];
        last_elem_ = &data_[tail_block_]->buf[(size - 1) % b];
    }

    Deque(std::initializer_list<int> list) {
        std::size_t b = kBlockSize / sizeof(int);
        if (list.size() == 0) {
            capacity_ = 2;
            data_ = std::make_unique<std::unique_ptr<Block>[]>(capacity_);
            size_ = 0;
            first_elem_ = nullptr;
            last_elem_ = nullptr;
            used_blocks_ = 0;
            head_block_ = 0;
            tail_block_ = 0;
            return;
        }
        std::size_t needed_blocks = (list.size() + b - 1) / b;
        capacity_ = 2;
        while (capacity_ < needed_blocks) {
            capacity_ *= 2;
        }
        data_ = std::make_unique<std::unique_ptr<Block>[]>(capacity_);
        for (std::size_t i = 0; i < capacity_; ++i) {
            data_[i] = nullptr;
        }

        auto all = list.begin();
        for (std::size_t i = 0; i < needed_blocks; ++i) {
            data_[i] = std::make_unique<Block>();
            std::size_t need = b < static_cast<std::size_t>(std::distance(all, list.end()))
                                   ? b
                                   : static_cast<std::size_t>(std::distance(all, list.end()));
            for (std::size_t j = 0; j < need; ++j, ++all) {
                data_[i]->buf[j] = *all;
            }
        }
        used_blocks_ = needed_blocks;
        head_block_ = 0;
        tail_block_ = used_blocks_ - 1;
        size_ = list.size();
        first_elem_ = &data_[head_block_]->buf[0];
        last_elem_ = &data_[tail_block_]->buf[(list.size() - 1) % b];
    }

    Deque& operator=(Deque rhs) {
        if (this != &rhs) {
            Swap(rhs);
        }
        return *this;
    }

    void Swap(Deque& rhs) {
        std::swap(data_, rhs.data_);
        std::swap(size_, rhs.size_);
        std::swap(capacity_, rhs.capacity_);
        std::swap(first_elem_, rhs.first_elem_);
        std::swap(last_elem_, rhs.last_elem_);
        std::swap(used_blocks_, rhs.used_blocks_);
        std::swap(head_block_, rhs.head_block_);
        std::swap(tail_block_, rhs.tail_block_);
    }

    void PushBack(int value) {
        std::size_t b = kBlockSize / sizeof(int);
        if (size_ == 0) {
            if (used_blocks_ == 0) {
                if (used_blocks_ == capacity_) {
                    Grow();
                }
                head_block_ = 0;
                tail_block_ = 0;
                if (!data_[0]) {
                    data_[0] = std::make_unique<Block>();
                }
                used_blocks_ = 1;
            }
            first_elem_ = last_elem_ = &data_[tail_block_]->buf[0];
            *last_elem_ = value;
            ++size_;
            return;
        }
        std::size_t pos = static_cast<std::size_t>(last_elem_ - data_[tail_block_]->buf);
        if (pos + 1 < b) {
            ++last_elem_;
            *last_elem_ = value;
        } else {
            if (used_blocks_ == capacity_) {
                Grow();
            }
            std::size_t new_tail = (tail_block_ + 1) % capacity_;
            if (!data_[new_tail]) {
                data_[new_tail] = std::make_unique<Block>();
            }
            tail_block_ = new_tail;
            ++used_blocks_;
            last_elem_ = &data_[tail_block_]->buf[0];
            *last_elem_ = value;
        }
        ++size_;
    }

    void PopBack() {
        std::size_t b = kBlockSize / sizeof(int);
        if (size_ == 0) {
            return;
        }
        if (size_ == 1) {
            data_[head_block_].reset();
            size_ = 0;
            used_blocks_ = 0;
            head_block_ = 0;
            tail_block_ = 0;
            first_elem_ = nullptr;
            last_elem_ = nullptr;
            return;
        }
        std::size_t pos = static_cast<std::size_t>(last_elem_ - data_[tail_block_]->buf);
        if (pos > 0) {
            --last_elem_;
        } else {
            data_[tail_block_].reset();
            --used_blocks_;
            tail_block_ = (tail_block_ + capacity_ - 1) % capacity_;
            std::size_t head = static_cast<std::size_t>(first_elem_ - data_[head_block_]->buf);
            std::size_t block = (head + (size_ - 2)) / b;
            std::size_t numblock = (head + (size_ - 2)) % b;
            std::size_t ind = (head_block_ + block) % capacity_;
            last_elem_ = &data_[ind]->buf[numblock];
        }
        size_--;
    }

    void PushFront(int value) {
        std::size_t b = kBlockSize / sizeof(int);
        if (size_ == 0) {
            if (used_blocks_ == 0) {
                if (used_blocks_ == capacity_) {
                    Grow();
                }
                head_block_ = 0;
                tail_block_ = 0;
                if (!data_[0]) {
                    data_[0] = std::make_unique<Block>();
                }
                used_blocks_ = 1;
            }
            first_elem_ = last_elem_ = &data_[head_block_]->buf[0];
            *first_elem_ = value;
            ++size_;
            return;
        }
        std::size_t pos = static_cast<std::size_t>(first_elem_ - data_[head_block_]->buf);
        if (pos > 0) {
            --first_elem_;
            *first_elem_ = value;
        } else {
            if (used_blocks_ == capacity_) {
                Grow();
            }
            std::size_t new_head = (head_block_ + capacity_ - 1) % capacity_;
            if (!data_[new_head]) {
                data_[new_head] = std::make_unique<Block>();
            }
            head_block_ = new_head;
            ++used_blocks_;
            first_elem_ = &data_[head_block_]->buf[b - 1];
            *first_elem_ = value;
        }
        ++size_;
    }

    void PopFront() {
        std::size_t b = kBlockSize / sizeof(int);
        if (size_ == 0) {
            return;
        }
        if (size_ == 1) {
            data_[head_block_].reset();
            size_ = 0;
            used_blocks_ = 0;
            head_block_ = 0;
            tail_block_ = 0;
            first_elem_ = nullptr;
            last_elem_ = nullptr;
            return;
        }
        std::size_t pos = static_cast<std::size_t>(first_elem_ - data_[head_block_]->buf);
        if (pos + 1 < b) {
            ++first_elem_;
        } else {
            data_[head_block_].reset();
            --used_blocks_;
            head_block_ = (head_block_ + 1) % capacity_;
            first_elem_ = &data_[head_block_]->buf[0];
        }
        size_--;
    }

    int& operator[](std::size_t ind) {
        std::size_t b = kBlockSize / sizeof(int);
        std::size_t head = first_elem_ - data_[head_block_]->buf;
        std::size_t blocknum = (head + ind) / b;
        std::size_t num_in_block = (head + ind) % b;
        std::size_t block = (head_block_ + blocknum) % capacity_;
        return data_[block]->buf[num_in_block];
    }

    const int& operator[](std::size_t ind) const {
        std::size_t b = kBlockSize / sizeof(int);
        std::size_t head = first_elem_ - data_[head_block_]->buf;
        std::size_t blocknum = (head + ind) / b;
        std::size_t num_in_block = (head + ind) % b;
        std::size_t block = (head_block_ + blocknum) % capacity_;
        return data_[block]->buf[num_in_block];
    }

    std::size_t Size() const {
        return size_;
    }

    void Clear() {
        for (std::size_t i = 0; i < capacity_; ++i) {
            data_[i].reset();
        }
        size_ = 0;
        first_elem_ = nullptr;
        last_elem_ = nullptr;
        used_blocks_ = 0;
        head_block_ = 0;
        tail_block_ = 0;
    }
};
