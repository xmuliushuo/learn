#include "list.h"
#include <stdio.h>
#include <stdlib.h>

struct vod_data {
	struct list_head fifo_list; // requests on the list sorted by the time 
};

struct request {
	struct list_head queue;
	int timeout;
};

void display(struct vod_data *vd);

int main()
{
	struct vod_data vd;
	struct request *rq = NULL;
	struct request *_rq, *temprq;
	int temp;
	int found = 0;
	INIT_LIST_HEAD(&vd.fifo_list);
	while (1) {
		printf("input an int:\n");
		scanf("%d", &temp);
		if (temp == -1) break;
		rq = (struct request *)malloc(sizeof(*rq));
		if (rq == NULL) {
			printf("malloc error\n");
			exit(1);
		}
		rq->timeout = temp;
		//list_add_tail(&rq->queue, &vd.fifo_list);
		found = 0;
		list_for_each_entry_safe_reverse(_rq, temprq, &vd.fifo_list, queue) {
			// printf("%d ", _rq->timeout);
			if (_rq->timeout <= temp) {
				found = 1;
				break;
			}
		}
		if (found == 1)
			list_add(&rq->queue, &_rq->queue);
		else
			list_add(&rq->queue, &vd.fifo_list);
		if (rq->queue.next != &vd.fifo_list) {
			_rq = list_entry(rq->queue.next, struct request, queue);
			if (rq->timeout >= (_rq->timeout - 1)) --rq->timeout;
		}
		temp = rq->timeout;
		printf("temp: %d\n", temp);
		if (rq->queue.prev != &vd.fifo_list) {
			_rq = list_entry(rq->queue.prev, struct request, queue);
			while (1) {
				if (_rq->timeout >= (temp - 1)) {
					_rq->timeout = temp - 2;
					temp -= 2;
					if (_rq->queue.prev == &vd.fifo_list) break;
					_rq = list_entry(_rq->queue.prev, struct request, queue);
				}
				else break;
			}
		}
		printf("\n");
		display(&vd);
	}
	return 0;
}

void display(struct vod_data *vd)
{
	struct request *rq, *temp;
	//rq = list_first_entry(&vd->fifo_list, struct request, queue);
	//list_for_each_entry(rq, &vd->fifo_list, queue) {
	printf("front: ");
	list_for_each_entry_safe(rq, temp, &vd->fifo_list, queue) {
		printf("%d ", rq->timeout);
	}
	printf("\nreverse: ");
	list_for_each_entry_safe_reverse(rq, temp, &vd->fifo_list, queue) {
		printf("%d ", rq->timeout);
	}
	printf("\n");
}
